<!--
Copyright (c) 2023-2024 SG Wireless - All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
copies  of  the  Software,  and  to  permit  persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Description: FUOTA (Firmware Update Over The Air) Module Documentation
-->

# FUOTA Module - Firmware Update Over The Air

The `fuota` module provides Python APIs for managing OTA (Over-The-Air) firmware updates on ESP32 devices.

## Features

- ✅ **Simple one-line upgrade**: `fuota.upgrade(url)` handles everything
- ✅ **Non-blocking mode**: Background OTA with status monitoring
- ✅ **Blocking mode**: Traditional synchronous operation
- ✅ **HTTPS support**: Built-in certificate bundle for secure downloads
- ✅ **Automatic partition management**: Selects next available OTA partition
- ✅ **Validation**: Verifies firmware image before flashing
- ✅ **Low memory usage**: Runs in C, no 64KB Python thread stack needed
- ✅ **Progress tracking**: Real-time status and progress monitoring
- ✅ **Percentage logging**: 1% increment progress updates
- ✅ **Network detection**: Automatic WiFi/LTE connectivity check
- ✅ **Automatic validation**: Validates new firmware on first boot
- ✅ **Soft reset safe**: Cleanly cancels OTA on Ctrl+D
- ✅ **Resume on network drops**: HTTP `Range` resume across TCP/TLS
  disconnects (ideal for NB-IoT / LTE-M / flaky Wi-Fi)
- ✅ **Exponential backoff retries**: Bounded retries with configurable
  backoff so the radio / NAT gets time to recover
- ✅ **Artifact identity check**: `ETag` / `Last-Modified` / total-size
  validated on every resume; server-side rebuilds force a clean restart
- ✅ **Optional chunk cap**: Reconnect every N bytes to sidestep carrier
  NAT / PSM timeouts even when data is flowing

## Resume Behaviour (NB-IoT / LTE-M)

The download loop keeps the ESP-IDF OTA handle (and its erased target
partition) alive across transport errors. When a read fails the HTTP client
is closed cleanly, the task sleeps with exponential backoff, then reconnects
with a `Range: bytes=<offset>-` request and keeps appending to the same
partition. If the server ignores `Range` (returns HTTP 200) or the artifact
identity changes mid-download (new `ETag` / `Last-Modified` / size) the
upgrade automatically restarts from byte 0.

Transient conditions that now auto-recover instead of failing the upgrade:
TLS read errors (`MBEDTLS_ERR_NET_CONN_RESET`, `ENOTCONN` on the HTTP
socket), `esp_http_client_open` failures, premature EOF, HTTP 5xx
responses, transient DNS failures.

Non-retryable conditions that still fail fast: HTTP 4xx, image validation
(`ESP_ERR_OTA_VALIDATE_FAILED`), no OTA partition available, memory
allocation failure for the RX buffer or OTA begin, `esp_ota_write` failure.

## API Reference

### `fuota.upgrade(url [, opts])`

**BLOCKING MODE** - Downloads and installs firmware synchronously.

**Parameters:**
- `url` (str, required): HTTPS/HTTP URL to firmware binary
- `opts` (dict, optional): Tunables. See *Options* below.

**Returns:**
- `True` on success, `False` on failure

**Blocks until complete** - REPL will be frozen during download and installation.

**Example:**
```python
import fuota
import machine

# Blocking upgrade - waits for completion
if fuota.upgrade('https://example.com/firmware.bin'):
    print("Success!")
    machine.reset()
else:
    print("Failed!")
```

---

### `fuota.start_upgrade(url [, opts])`

**NON-BLOCKING MODE (RECOMMENDED)** - Downloads and installs firmware in background.

**Parameters:**
- `url` (str, required): HTTPS/HTTP URL to firmware binary
- `opts` (dict, optional): Tunables. See *Options* below.

**Returns:**
- `None` - Returns immediately, use `fuota.status()` to monitor progress

**Raises:**
- `OSError`: If OTA already in progress, network error, or failed to create task

**Example:**
```python
import fuota
import time
import machine

# Start OTA in background - returns immediately, tuned for NB-IoT
fuota.start_upgrade('https://example.com/firmware.bin', {
    'max_retries': 30,
    'timeout_ms': 90000,
    'max_chunk_bytes': 131072,   # reconnect every 128 KB
})

# Monitor progress
while True:
    status = fuota.status()
    print("state={} attempt={} offset={}/{} ({}%)".format(
        status['state'],
        status.get('attempts', 0),
        status['bytes_written'],
        status.get('total_size', '?'),
        status.get('percent', '?'),
    ))

    if status['state'] == 'success':
        print("Upgrade successful!")
        machine.reset()
        break
    elif status['state'] == 'failed':
        print("Upgrade failed: {}".format(status.get('err_msg', 'Unknown error')))
        break

    time.sleep(1)
```

### Options

All keys are optional; defaults are shown in parentheses.

| Key                   | Default | Description |
|-----------------------|---------|-------------|
| `max_retries`         | 20      | Total HTTP attempts, including the first. |
| `backoff_initial_ms`  | 2000    | Delay before the first retry. Doubled on each subsequent failure, capped by `backoff_max_ms`. |
| `backoff_max_ms`      | 60000   | Upper bound on the exponential backoff delay. |
| `timeout_ms`          | 60000   | Per-HTTP-request timeout (read / connect). |
| `rx_buffer_size`      | 4096    | Receive buffer + OTA write block size (clamped to 512–16384). |
| `max_chunk_bytes`     | 0       | 0 = unlimited. Non-zero caps how much data is pulled per HTTP connection; the next `Range` request resumes from where it left off. Useful to defeat carrier NAT / PSM idle timeouts on long downloads. |

---

### `fuota.status()`

Returns current OTA operation status (for non-blocking mode).

**Returns:**
Dictionary with keys:
- `state` (str): `'idle'`, `'running'`, `'success'`, `'failed'`, or `'aborted'`
- `bytes_written` (int): Bytes downloaded so far
- `attempts` (int): Number of HTTP attempts used so far (1 on the first try)
- `resume_offset` (int): Offset of the most recent HTTP attempt
- `total_size` (int, optional): Total firmware size (if known)
- `percent` (int, optional): 0-100 progress (if `total_size` is known)
- `error_code` (int, optional): ESP-IDF error code (on failure)
- `err_msg` (str, optional): Error description (on failure)

**Example:**
```python
>>> fuota.status()
{'state': 'running', 'bytes_written': 524288, 'total_size': 2621440,
 'percent': 20, 'attempts': 3, 'resume_offset': 458752}

>>> fuota.status()
{'state': 'failed', 'bytes_written': 100000, 'attempts': 20,
 'resume_offset': 98304, 'error_code': -1,
 'err_msg': 'max retries exceeded [ESP_FAIL]'}
```

---

### `fuota.abort_upgrade()`

Cancels an ongoing OTA upgrade (non-blocking mode). The cancellation is
cooperative: the download task notices the request at the next read / retry
boundary, cleanly closes the HTTP client and releases the OTA partition
handle. `fuota.status()` will transition to `'aborted'` shortly after the
call returns.

**Raises:**
- `OSError`: If no OTA upgrade is in progress

**Example:**
```python
fuota.start_upgrade('https://example.com/firmware.bin')
time.sleep(5)
fuota.abort_upgrade()  # Cancel after 5 seconds
```

---

### `fuota.info()`

Prints current partition information to console.

**Example:**
```python
>>> fuota.info()
Next update partition: ota_1
Running partition: ota_0
Boot partition: ota_0
```

---

### `fuota.partition_info()`

Returns partition information as a named tuple (use this instead of `info()` for programmatic access).

**Returns:**
Named tuple with fields:
- `next_update_partition` (str): Partition for next OTA
- `running_partition` (str): Currently running partition
- `boot_partition` (str): Partition system booted from

**Example:**
```python
>>> info = fuota.partition_info()
>>> print(f"Running: {info.running_partition}, Next: {info.next_update_partition}")
Running: ota_0, Next: ota_1

>>> info.running_partition
'ota_0'
```

---

### `fuota.valid()`

Marks current firmware as valid, preventing automatic rollback.

**Important:** Call this after validating new firmware works correctly!

---

### `fuota.rollback()`

Marks current firmware as invalid and reboots to previous version.

---

### Low-Level API (Advanced)

### `fuota.start()`

Begins manual OTA process.

### `fuota.write(data)`

Writes firmware data chunk.

**Parameters:**
- `data` (bytes): Firmware data chunk

### `fuota.finish()`

Completes manual OTA and sets boot partition.

## Usage Patterns

### Pattern 1: Non-Blocking Upgrade with Progress (Recommended)

```python
import fuota
import time
import machine

url = 'https://example.com/firmware.bin'

# Start OTA in background
fuota.start_upgrade(url)
print("OTA started in background...")

# Monitor progress
while True:
    status = fuota.status()
    state = status['state']
    
    if state == 'running':
        bytes_written = status['bytes_written']
        total = status.get('total_size')
        if total:
            pct = (bytes_written * 100) // total
            print(f"Downloading: {pct}% ({bytes_written}/{total} bytes)")
        else:
            print(f"Downloading: {bytes_written} bytes")
    
    elif state == 'success':
        print("✓ Upgrade successful! Rebooting...")
        time.sleep(1)
        machine.reset()
        break
    
    elif state == 'failed':
        print(f"✗ Upgrade failed: {status.get('err_msg', 'Unknown error')}")
        print(f"  Error code: {status.get('error_code')}")
        break
    
    elif state == 'aborted':
        print("OTA was aborted")
        break
    
    time.sleep(1)
```

### Pattern 2: Simple Blocking Upgrade

```python
import fuota
import machine

try:
    # Blocks until complete
    if fuota.upgrade('https://example.com/firmware.bin'):
        print("Upgrade successful, rebooting...")
        machine.reset()
    else:
        print("Upgrade failed")
except OSError as e:
    print(f"Error: {e}")
```

### Pattern 3: User-Cancelable Upgrade

```python
import fuota
import time
import sys

url = 'https://example.com/firmware.bin'
fuota.start_upgrade(url)

print("Upgrading... Press Ctrl+C to cancel")

try:
    while True:
        status = fuota.status()
        if status['state'] in ('success', 'failed', 'aborted'):
            break
        time.sleep(0.5)
        
except KeyboardInterrupt:
    print("\nCanceling upgrade...")
    fuota.abort_upgrade()
    print("Upgrade canceled")
```

### Pattern 4: Safe Upgrade with Automatic Validation

The firmware automatically validates on first boot after OTA! Just reset after successful upgrade:

```python
import fuota
import machine

# No need to manually call fuota.valid() anymore!
if fuota.upgrade('https://example.com/firmware.bin'):
    print("Rebooting to new firmware...")
    machine.reset()  # New firmware auto-validates on boot
```

### Pattern 5: Safe Upgrade with Manual Validation

```python
import fuota
import machine

# Check current state
info = fuota.partition_info()
print(f"Current partition: {info.running_partition}")

# Perform upgrade
if fuota.upgrade('https://example.com/firmware.bin'):
    print("Upgrade complete, rebooting...")
    machine.reset()

# After reboot, in boot.py or main.py:
def validate_firmware():
    """Test critical functionality"""
    try:
        # Test your critical features
        import important_module
        important_module.test()
        return True
    except Exception as e:
        print(f"Validation failed: {e}")
        return False

if validate_firmware():
    print("✓ Firmware validated")
    fuota.valid()  # Mark as valid
else:
    print("✗ Validation failed, rolling back...")
    fuota.rollback()  # Reboot to previous version
```

## Memory Usage

The new `fuota.upgrade()` function is implemented in C and uses the ESP-IDF's optimized HTTPS OTA stack:

- **No Python thread needed**: Runs directly in main thread
- **No 64KB stack allocation**: Uses default C stack
- **Efficient streaming**: Downloads and flashes in chunks
- **PSRAM available**: All 8MB PSRAM remains free for other uses

### Old vs New Approach

**Old (Python thread with urequests):**
```python
_thread.start_new_thread(download_thread, ())  # 64KB internal RAM!
```
- Required 64KB thread stack in internal RAM
- Used Python HTTP libraries (slower, more memory)
- Complex error handling

**New (C implementation):**
```python
fuota.upgrade(url)  # No extra stack needed
```
- Uses default C stack
- ESP-IDF optimized HTTP/TLS stack
- Automatic error handling

## Network Requirements

- Active WiFi or LTE connection
- DNS resolution for HTTPS URLs
- Sufficient bandwidth for firmware download
- Stable connection (30s - several minutes depending on size)

## Security

- **HTTPS recommended**: Use certificate bundle or custom cert
- **Firmware validation**: Automatic integrity checking
- **Rollback protection**: Mark firmware valid after testing
- **Partition isolation**: Failed upgrades don't affect running firmware

## Troubleshooting

### "OTA upgrade already in progress"
- An OTA operation is already running
- Wait for it to complete or call `fuota.abort_upgrade()`

### "Network not connected - please connect to WiFi or LTE first"
- No active network connection detected
- Connect to WiFi or LTE before starting OTA
- Verify IP address is assigned

### "ESP HTTPS OTA Begin failed"
- Invalid URL or unreachable server
- Check URL is correct and server is running
- Test URL in browser first

### "Image validation failed, image is corrupted"
- Downloaded firmware is corrupted or incomplete
- Check network stability during download
- Verify firmware binary is valid for your hardware

### "Unable to read remote firmware descriptor"
- Server returned invalid firmware image
- Ensure URL points to actual firmware binary (not HTML page)
- Check firmware is built for ESP32-S3

### Slow Downloads
- Increase HTTP buffer size (requires code modification)
- Current: 128 bytes for fast flash writes
- Trade-off: Larger buffers = faster download but slower flash writes

### REPL Hangs During Blocking Mode
- Fixed in current version with `is_background_task` flag
- Ensure you're using latest firmware
- Use non-blocking mode if issues persist

### Partition Shows Wrong Subtype After Downgrade
- Flashing new firmware updates partition table
- Subtype changes from `6` (IDF v4) to `131/0x83` (IDF v5)
- This is normal - indicates which firmware version is active
- **Important**: Downgrading from IDF v5 to IDF v4 requires `--erase-flash` due to LittleFS version incompatibility

## Implementation Details

### Architecture

- **C-based implementation**: Uses ESP-IDF `esp_https_ota` API
- **Two distinct APIs**:
  - `fuota.upgrade(url)`: **Blocking** - runs in calling thread, returns after completion
  - `fuota.start_upgrade(url)`: **Non-blocking** - runs in FreeRTOS task (priority 5, 8KB stack)
- **Automatic partition selection**: Toggles between ota_0/ota_1
- **Built-in certificate bundle**: Support for common CA certificates
- **Configurable HTTP timeout**: Default 30 seconds
- **Efficient streaming**: 128-byte HTTP buffers for fast flash writes

### Progress Logging

- **Percentage-based**: Logs every 1% when total size known
- **Byte-based**: Logs every write when size unknown
- **Event-driven**: Uses ESP-IDF OTA event system
- **Debug output**: Comprehensive firmware metadata logging

### Network Detection

Automatically checks connectivity before starting:
1. Checks WiFi (WIFI_STA_DEF interface)
2. Falls back to LTE (PPP_DEF interface)  
3. Fails fast if no IP address available

### Automatic Validation

New firmware is automatically validated on first boot:
- Checks if running from OTA partition
- Detects pending validation state
- Calls `esp_ota_mark_app_valid_cancel_rollback()` automatically
- No manual `fuota.valid()` call needed (unless you want extra validation)

### Soft Reset Safety

When you press Ctrl+D (soft reset):
- Automatically cancels any running OTA operation
- Cleans up background task
- Prevents orphaned OTA tasks
- Safe to restart Python environment during OTA

### Task Priority

Background OTA task runs at priority 5 to:
- Avoid preemption during flash writes
- Ensure stable flash operations
- Allow REPL to remain responsive
- Priority higher than MicroPython main task (priority 1)

## Future Enhancements

- [x] Non-blocking mode with status monitoring
- [x] Progress tracking with percentage updates
- [x] Automatic firmware validation on boot
- [x] Soft reset safety (clean OTA cancellation)
- [x] Network connectivity detection
- [ ] Resume interrupted downloads
- [ ] Delta/differential updates
- [ ] Custom progress callbacks
- [ ] Configurable HTTP buffer size via API
- [ ] Multi-stage bootloader support

## See Also

- Example: `/examples/fuota/ota_example.py`
- ESP-IDF OTA docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html

<!--- end of file --->
