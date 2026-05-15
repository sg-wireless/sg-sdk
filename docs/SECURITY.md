<!--
Copyright (c) 2026 SG Wireless - All Rights Reserved

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
-->

# Firmware Security (Secure Boot + Flash Encryption)

## Contents

* [What `--secure` does](#what---secure-does)
* [Prerequisites](#prerequisites)
* [Secure boot signing key](#secure-boot-signing-key)
* [Build secured firmware](#build-secured-firmware)
* [First provisioning / flashing a secured device](#first-provisioning--flashing-a-secured-device)
* [Important downsides and operational constraints](#important-downsides-and-operational-constraints)
* [Build signed firmware for OTA](#build-signed-firmware-for-ota-applicationbin)
* [OTA update compatibility](#ota-update-compatibility)
* [Recommended process](#recommended-process)

This document explains how to build and flash F1 firmware with security enabled, what changes after provisioning a secured device, and how to produce OTA-updatable signed firmware images.
It is assumed that sg-sdk was cloned in your home directory ($HOME/sg-sdk).

## What `--secure` does

When you pass `--secure` to the firmware builder:

- The F1 secure config file `src/platforms/F1/configs/sdkconfig.secure` is included.
- Secure Boot V2 (RSA signed images) is enabled.
- Flash Encryption (AES-256, release mode) is enabled.
- If the command is `flash`, the builder internally upgrades it to a full-flash flow so the bootloader is also flashed.

## Prerequisites

Before using `--secure`, verify:

1. The `esp-idf` submodule is initialised (this happens automatically on the first `fw_builder.sh` build run). The signing key at `CONFIG_SECURE_BOOT_SIGNING_KEY` is auto-generated if absent.
2. Partition table includes `nvs_keys` (already done for F1).
3. You are using the correct board and serial port.

## Secure boot signing key

In `src/platforms/F1/configs/sdkconfig.secure`, the key path is:

```text
CONFIG_SECURE_BOOT_SIGNING_KEY="../../../secure_boot_signing_key.pem"
```

The builder runs ESP-IDF with working directory `src/platforms/F1`, so this resolves to:

```text
$HOME/sg-sdk/secure_boot_signing_key.pem
```

### Auto-generation on first use

When `--secure` is passed to `fw_builder.sh`, the builder automatically checks whether the signing key exists. If it does not, it generates one using `espsecure.py` before the build starts. This means no manual key-generation step is needed.

> **Note:** Auto-generation requires that the `esp-idf` submodule is already initialised, which happens automatically as part of the normal build flow the first time you run `./fw_builder.sh ... build`.

### Manual generation (optional)

If you prefer to generate the key yourself before the first build:

```bash
cd $HOME/sg-sdk
bash -c ". ext/esp-idf/export.sh && espsecure.py generate_signing_key --version 2 secure_boot_signing_key.pem"
```

Either way, store this private key securely and do not commit it to git.

## Build secured firmware

Example (MicroPython variant):

Please close your terminal after running the above command and start a new terminal.

```bash
cd $HOME/sg-sdk
./fw_builder.sh --board SGW3501-F1-StarterKit --secure build
```

This produces a security-enabled build where app images are signed for secure boot.

## First provisioning / flashing a secured device

Use `flash` together with `--secure`:

```bash
cd $HOME/sg-sdk
./fw_builder.sh --board SGW3501-F1-StarterKit --port /dev/ttyUSB0 --secure flash
```

Notes:

- With `--secure`, `flash` is internally converted to a full-flash operation.
- This includes bootloader flashing (required when enabling secure boot for the first time).

> ## ❗ After flashing: wait for internal encryption to complete
>
> After the firmware has been flashed, **do not disconnect or reset the device manually**. The device
> needs several minutes to complete its internal flash encryption process. Interrupting this process
> (e.g. by power-cycling or resetting the device prematurely) **may render the device permanently unusable**.
>
> **Wait until the device resets itself automatically** — this signals that encryption is complete.
>
> ### Normal errors during encryption
>
> While encryption is in progress, you may see error messages like the following in the serial output:
>
> ```
> E (27528) esp_image: image at 0x2a0000 has invalid magic byte (nothing flashed here?)
> E (27528) esp_image: image at 0x520000 has invalid magic byte (nothing flashed here?)
> ```
>
> These errors are **expected and harmless**. They occur because the OTA firmware image slots are
> empty (no OTA update has been applied yet). You can safely ignore them.

## Important downsides and operational constraints

After secure boot + flash encryption are enabled on a device:

- You can no longer treat the chip like a normal development target for arbitrary raw flashing.
- Generic `esptool.py write_flash` workflows become restricted/not practical for normal development updates.
- Debug and recovery options are reduced compared to insecure/development mode.
- If signing keys are lost, producing valid update images becomes impossible.
- **The NVS partition is erased** during the first boot after flashing. This includes all LoRaWAN
  provisioning parameters (DevEUI, AppEUI, AppKey, etc.) and the `devNonce` counter. The device
  will need to be re-provisioned after secure boot is enabled. See the
  [ESP-IDF NVS Encryption documentation](https://docs.espressif.com/projects/esp-idf/en/release-v5.4/esp32s3/api-reference/storage/nvs_encryption.html)
  for technical background on why this happens.

In short: once secured, firmware lifecycle must follow signed update paths.

## Build signed firmware for OTA (`application.bin`)

To generate OTA payloads, run a secure build:

```bash
cd $HOME/sg-sdk
./fw_builder.sh --board SGW3501-F1-StarterKit --secure build
```

The OTA image to distribute is the generated `application.bin` from the build directory, for example:

```text
build/sdk-default/F1/SGW3501-F1-StarterKit/micropython/application.bin
```

If you build from a custom project dir or variant, the path changes accordingly under `build/<app>/<platform>/<board>/<variant>/application.bin`.

## OTA update compatibility

For a secured device, OTA update images must be:

- Signed with the same trusted key chain expected by secure boot.
- Built with secure settings compatible with the running device.

If signature verification fails, the image will not boot.

## Recommended process

1. Keep signing keys in secure storage and back them up safely.
2. Use `--secure build` in CI for release artifacts.
3. Use `--secure flash` only for provisioning / controlled service flows.
4. Use OTA with signed `application.bin` for field updates.

<!--- end of file --->
