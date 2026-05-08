#!/usr/bin/env python3
# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
# copies  of  the  Software,  and  to  permit  persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
# IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author    SG Wireless
# ---------------------------------------------------------------------------- #

"""
MicroPython REPL Automated Test Script

A generic tool for automated testing of MicroPython functionality via serial REPL.
Supports test configuration files, expected response validation, and detailed logging.

Usage:
    python3 repl_tester.py --port /dev/ttyUSB0 --test-file test_usb.json
    python3 repl_tester.py --port /dev/ttyUSB0 --commands "import machine" "print(hasattr(machine, 'USBDevice'))"
    python3 repl_tester.py --port /dev/ttyUSB0 --interactive

Author: sg-sdk automated testing framework
Date: October 2025
"""

import argparse
import json
import serial
import time
import sys
import os
from typing import List, Dict, Any, Optional, Tuple
from dataclasses import dataclass


@dataclass
class TestCommand:
    """Represents a single test command with expected behavior"""
    command: str
    description: str = ""
    timeout: float = 2.0
    expected_response: Optional[str] = None
    expected_contains: Optional[List[str]] = None
    expected_not_contains: Optional[List[str]] = None
    should_succeed: bool = True


@dataclass
class TestResult:
    """Represents the result of executing a test command"""
    command: str
    success: bool
    response: str
    error_message: str = ""
    execution_time: float = 0.0


class REPLTester:
    """Automated MicroPython REPL testing framework"""
    
    def __init__(self, port: str, baudrate: int = 115200, debug: bool = False):
        """
        Initialize the REPL tester
        
        Args:
            port: Serial port (e.g., /dev/ttyUSB0)
            baudrate: Serial communication speed
            debug: Enable debug output
        """
        self.port = port
        self.baudrate = baudrate
        self.debug = debug
        self.ser: Optional[serial.Serial] = None
        
    def connect(self) -> bool:
        """
        Connect to the MicroPython device
        
        Returns:
            True if connection successful, False otherwise
        """
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=3)
            if self.debug:
                print(f"Connected to {self.port} at {self.baudrate} baud")
            
            # Wait for device to be ready
            time.sleep(2)
            
            # Send Ctrl+C to interrupt any running code
            self.ser.write(b'\x03')
            time.sleep(0.5)
            
            # Clear input buffer and get fresh prompt
            self.ser.reset_input_buffer()
            self.ser.write(b'\r\n')
            time.sleep(0.5)
            
            return True
            
        except Exception as e:
            print(f"Failed to connect to {self.port}: {e}")
            return False
    
    def disconnect(self):
        """Close the serial connection"""
        if self.ser:
            self.ser.close()
            self.ser = None
            if self.debug:
                print("Disconnected from device")
    
    def execute_command(self, test_cmd: TestCommand) -> TestResult:
        """
        Execute a single command and validate the response
        
        Args:
            test_cmd: TestCommand object with command and expectations
            
        Returns:
            TestResult with execution details
        """
        if not self.ser:
            return TestResult(
                command=test_cmd.command,
                success=False,
                response="",
                error_message="Not connected to device"
            )
        
        start_time = time.time()
        
        try:
            if self.debug:
                print(f"Executing: {test_cmd.command}")
            
            # Send command
            self.ser.write(f'{test_cmd.command}\r\n'.encode())
            
            # Read response with timeout
            response = ""
            end_time = start_time + test_cmd.timeout
            
            while time.time() < end_time:
                if self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
                    response += data
                time.sleep(0.1)
            
            execution_time = time.time() - start_time
            
            # Clean up response (remove echo and prompts)
            response_lines = response.split('\n')
            clean_lines = []
            for line in response_lines:
                clean_line = line.strip()
                if clean_line and not clean_line.startswith('>>>') and not clean_line.startswith('...'):
                    clean_lines.append(clean_line)
            
            clean_response = '\n'.join(clean_lines)
            
            # Validate response
            success, error_msg = self._validate_response(test_cmd, clean_response)
            
            return TestResult(
                command=test_cmd.command,
                success=success,
                response=clean_response,
                error_message=error_msg,
                execution_time=execution_time
            )
            
        except Exception as e:
            return TestResult(
                command=test_cmd.command,
                success=False,
                response="",
                error_message=f"Exception during execution: {e}",
                execution_time=time.time() - start_time
            )
    
    def _validate_response(self, test_cmd: TestCommand, response: str) -> Tuple[bool, str]:
        """
        Validate command response against expectations
        
        Args:
            test_cmd: TestCommand with expectations
            response: Actual response from device
            
        Returns:
            Tuple of (success, error_message)
        """
        # Check exact response match
        if test_cmd.expected_response is not None:
            if response.strip() != test_cmd.expected_response.strip():
                return False, f"Expected exact response '{test_cmd.expected_response}', got '{response}'"
        
        # Check required content
        if test_cmd.expected_contains:
            for required in test_cmd.expected_contains:
                if required not in response:
                    return False, f"Response missing required content: '{required}'"
        
        # Check forbidden content
        if test_cmd.expected_not_contains:
            for forbidden in test_cmd.expected_not_contains:
                if forbidden in response:
                    return False, f"Response contains forbidden content: '{forbidden}'"
        
        return True, ""
    
    def run_test_suite(self, test_commands: List[TestCommand]) -> List[TestResult]:
        """
        Run a complete test suite
        
        Args:
            test_commands: List of TestCommand objects
            
        Returns:
            List of TestResult objects
        """
        results = []
        
        print(f"Running {len(test_commands)} test commands...")
        
        for i, test_cmd in enumerate(test_commands, 1):
            print(f"\n[{i}/{len(test_commands)}] {test_cmd.description or test_cmd.command}")
            
            result = self.execute_command(test_cmd)
            results.append(result)
            
            # Print result
            status = "✅ PASS" if result.success else "❌ FAIL"
            print(f"  {status} ({result.execution_time:.2f}s)")
            
            if not result.success:
                print(f"  Error: {result.error_message}")
            
            if self.debug and result.response:
                print(f"  Response: {repr(result.response)}")
        
        return results
    
    def load_test_file(self, filepath: str) -> List[TestCommand]:
        """
        Load test commands from JSON file
        
        Args:
            filepath: Path to JSON test file
            
        Returns:
            List of TestCommand objects
        """
        try:
            with open(filepath, 'r') as f:
                data = json.load(f)
            
            test_commands = []
            for cmd_data in data.get('commands', []):
                test_cmd = TestCommand(
                    command=cmd_data['command'],
                    description=cmd_data.get('description', ''),
                    timeout=cmd_data.get('timeout', 2.0),
                    expected_response=cmd_data.get('expected_response'),
                    expected_contains=cmd_data.get('expected_contains'),
                    expected_not_contains=cmd_data.get('expected_not_contains'),
                    should_succeed=cmd_data.get('should_succeed', True)
                )
                test_commands.append(test_cmd)
            
            return test_commands
            
        except Exception as e:
            print(f"Failed to load test file {filepath}: {e}")
            return []
    
    def interactive_mode(self):
        """Run in interactive mode for manual testing"""
        print("Interactive REPL mode. Type 'exit' to quit.")
        print("Commands are sent directly to the device.\n")
        
        while True:
            try:
                command = input(">>> ").strip()
                if command.lower() in ['exit', 'quit']:
                    break
                
                if command:
                    test_cmd = TestCommand(command=command, timeout=5.0)
                    result = self.execute_command(test_cmd)
                    if result.response:
                        print(result.response)
                    if not result.success and result.error_message:
                        print(f"Error: {result.error_message}")
                        
            except KeyboardInterrupt:
                print("\nExiting interactive mode...")
                break
            except EOFError:
                break
    
    def print_summary(self, results: List[TestResult]):
        """Print test suite summary"""
        total = len(results)
        passed = sum(1 for r in results if r.success)
        failed = total - passed
        
        print(f"\n{'='*50}")
        print(f"TEST SUMMARY")
        print(f"{'='*50}")
        print(f"Total:  {total}")
        print(f"Passed: {passed} ✅")
        print(f"Failed: {failed} ❌")
        print(f"Success Rate: {(passed/total*100):.1f}%")
        
        if failed > 0:
            print(f"\nFailed Tests:")
            for result in results:
                if not result.success:
                    print(f"  • {result.command}")
                    print(f"    {result.error_message}")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="MicroPython REPL Automated Test Script")
    parser.add_argument("--port", "-p", required=True, help="Serial port (e.g., /dev/ttyUSB0)")
    parser.add_argument("--baudrate", "-b", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--test-file", "-f", help="JSON file with test commands")
    parser.add_argument("--commands", "-c", nargs="+", help="Direct commands to execute")
    parser.add_argument("--interactive", "-i", action="store_true", help="Interactive mode")
    parser.add_argument("--debug", "-d", action="store_true", help="Enable debug output")
    parser.add_argument("--timeout", "-t", type=float, default=2.0, help="Default command timeout (default: 2.0s)")
    
    args = parser.parse_args()
    
    if not any([args.test_file, args.commands, args.interactive]):
        parser.error("Must specify --test-file, --commands, or --interactive")
    
    # Initialize tester
    tester = REPLTester(args.port, args.baudrate, args.debug)
    
    try:
        # Connect to device
        if not tester.connect():
            sys.exit(1)
        
        if args.interactive:
            # Interactive mode
            tester.interactive_mode()
            
        elif args.test_file:
            # Load and run test file
            test_commands = tester.load_test_file(args.test_file)
            if not test_commands:
                print("No test commands loaded")
                sys.exit(1)
            
            results = tester.run_test_suite(test_commands)
            tester.print_summary(results)
            
            # Exit with error code if any tests failed
            if any(not r.success for r in results):
                sys.exit(1)
                
        elif args.commands:
            # Direct command execution
            test_commands = [
                TestCommand(command=cmd, timeout=args.timeout)
                for cmd in args.commands
            ]
            
            results = tester.run_test_suite(test_commands)
            tester.print_summary(results)
            
            # Exit with error code if any tests failed
            if any(not r.success for r in results):
                sys.exit(1)
    
    finally:
        tester.disconnect()


if __name__ == "__main__":
    main()