<!------------------------------------------------------------------------------
 ! @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
 !
 ! Permission is hereby granted, free of charge, to any person obtaining a copy
 ! of this software and associated documentation files(the "Software"), to deal
 ! in the Software without restriction, including without limitation the rights
 ! to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 ! copies  of  the  Software,  and  to  permit  persons to whom the Software is
 ! furnished to do so, subject to the following conditions:
 !
 ! The above copyright notice and this permission notice shall be included in
 ! all copies or substantial portions of the Software.
 !
 ! THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 ! IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 ! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 ! AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 ! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 ! OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 ! THE SOFTWARE.
 !
 ! @author  Ahmed Sabry (SG Wireless)
 !
 ! @brief   Documentation file for Non-Volatile Storage Interface.
 !----------------------------------------------------------------------------->

# Non-Volatile Storage (NVS) Interface

<!------------------------------------------------------------------------------
 ! TOC
 !----------------------------------------------------------------------------->

## Contents

* [Introduction](#intro)
* [NVS Statistics](#stat)
* [NVS Query](#query)
* [NVS Key-Value Operations](#kv-ops)

<!------------------------------------------------------------------------------
 ! Introduction
 !----------------------------------------------------------------------------->
<div id="intro"></div>

## Introduction

The NVS (Non-Volatile Storage) interface provides access to the ESP32's NVS
flash memory system. NVS is used to persistently store key-value pairs,
configuration data, and other application state that must survive system
resets and power cycles.

The NVS interface component exposes the underlying ESP NVS library functionality
through a micropython module, allowing applications to read, write, and manage
data stored in dedicated NVS partitions.

<!------------------------------------------------------------------------------
 ! NVS Statistics
 !----------------------------------------------------------------------------->
<div id="stat"></div>

## NVS Statistics

`nvs_if.stat()` displays comprehensive statistics about the NVS partitions
and their contents. It can be called with optional filters to view specific
partitions or namespaces.

### Function Signature

```BNF
(1) nvs_if.stat()
(2) nvs_if.stat( dump_blobs=<bool> )
(3) nvs_if.stat( partition=<name>, namespace=<name>, dump_blobs=<bool> )
```

### Parameters

* `dump_blobs` (bool, default: False) - If True, displays hexadecimal content
  of BLOB entries in addition to their size.
* `partition` (str, optional) - Filter output to a specific NVS partition
  (e.g., "nvs"). If omitted, all partitions are displayed.
* `namespace` (str, optional) - Filter output to a specific namespace within
  the partition(s).

### Output Format

The output displays all key-value pairs organized by partition and namespace,
with the following information for each entry:

* **namespace** - The namespace the key belongs to
* **key** - The key name
* **type** - The data type (U8, I32, STR, BLOB, etc.)
* **val** - The current value or size (for BLOBs)

At the end of each partition, a summary is displayed showing:

* **partition size** - Total capacity of the NVS partition in bytes
* **used entries** - Number of entries currently used / total available entries
* **free** - Percentage of available free space

### Examples

```python
import nvs_if

# Display all NVS statistics for all partitions
nvs_if.stat()

# Display statistics for a specific partition
nvs_if.stat(partition="nvs")

# Display statistics for a specific namespace
nvs_if.stat(namespace="app")

# Display all entries with BLOB hex data
nvs_if.stat(dump_blobs=True)
```

### Example Output

```
====================================================================================================
namespace   key              type    val                                                            
====================================================================================================

---( nvs )------------------------------------------------------------------------------------------
ctrl        ztp              U8      0                                                               
lora-stack  lora-mgr         BLOB    size: 12
lora-stack  lw-radio-proc    BLOB    size: 20
app         frame            STR     Frame #3                                                       
app         iframe           I32     3                                                               
phy         cal_data         BLOB    size: 1904
phy         cal_version      U32     540                                                               

    partition size : 24576 bytes
    used entries   : 32 / 126
    free           : 74%
====================================================================================================
```

<!------------------------------------------------------------------------------
 ! NVS Query
 !----------------------------------------------------------------------------->
<div id="query"></div>

## NVS Query

`nvs_if.exists()` checks whether a key exists in NVS storage, either in a
specific partition and namespace or in one of the default partitions.

### Function Signature

```BNF
nvs_if.exists( key, namespace, partition=<name> )
```

### Parameters

* `key` (str) - The key name to check for existence. If None, checks for
  namespace existence only.
* `namespace` (str) - The namespace in which to search.
* `partition` (str, optional) - The NVS partition name. If omitted, searches
  the default "nvs" partition.

### Return Value

* **True** if the key (or namespace) exists
* **False** if the key (or namespace) does not exist

### Examples

```python
import nvs_if

# Check if a key exists in the default partition
if nvs_if.exists(key="ztp", namespace="ctrl"):
    print("ZTP key found")
else:
    print("ZTP key not found")

# Check namespace existence
if nvs_if.exists(key=None, namespace="app"):
    print("app namespace exists")

# Check in a specific partition
if nvs_if.exists(key="data", namespace="myapp", partition="nvs"):
  print("data key found in nvs partition")
```

<!------------------------------------------------------------------------------
 ! NVS Key-Value Operations
 !----------------------------------------------------------------------------->
<div id="kv-ops"></div>

## NVS Key-Value Operations

`nvs_if.set()` stores or updates a key-value pair in NVS storage.

### Function Signature

```BNF
nvs_if.set( key, value, namespace, partition=<name> )
```

### Parameters

* `key` (str) - The key name
* `value` - The value to store (bytes, int, or str, depending on intended type)
* `namespace` (str) - The namespace in which to store the key
* `partition` (str, optional) - The NVS partition name. If omitted, uses the
  default "nvs" partition

### Return Value

* **True** if the operation was successful
* **False** if the operation failed

### Examples

```python
import nvs_if

# Store an integer value
nvs_if.set(key="counter", value=42, namespace="app")

# Store a string value
nvs_if.set(key="ssid", value="WiFiNetwork", namespace="config")

# Store binary data (BLOB)
nvs_if.set(key="data", value=b"\x01\x02\x03\x04", namespace="app")

# Store in a specific partition
nvs_if.set(key="setting", value=100, namespace="app", partition="nvs")
```

### Reading Values

`nvs_if.get()` retrieves a stored value from NVS storage.

### Function Signature

```BNF
nvs_if.get( key, namespace, partition=<name>, default=None )
```

### Parameters

* `key` (str) - The key name to retrieve
* `namespace` (str) - The namespace containing the key
* `partition` (str, optional) - The NVS partition name. If omitted, searches
  the default "nvs" partition
* `default` (optional) - Value to return if the key is not found. If not
  specified and key doesn't exist, raises an exception.

### Return Value

The stored value, or the default value if the key doesn't exist.

### Examples

```python
import nvs_if

# Retrieve a value
counter = nvs_if.get(key="counter", namespace="app")
print(f"Counter: {counter}")

# Retrieve with a default value
ssid = nvs_if.get(key="ssid", namespace="config", default="DefaultSSID")

# Retrieve from a specific partition
setting = nvs_if.get(key="setting", namespace="app", partition="nvs")
```

<!--- end of file ------------------------------------------------------------->
