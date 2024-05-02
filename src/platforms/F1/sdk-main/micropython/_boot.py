import gc
import os

try:
    from flashbdev import bdev
    if bdev:
        os.mount(bdev, "/")
except OSError:
    import _inisetup
    vfs = _inisetup.setup()

# Change the working directory to /flash to be compatible with the legacy FW
gc.collect()