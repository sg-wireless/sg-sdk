import gc
import os

try:
    from flashbdev import bdev
    if bdev:
        os.mount(bdev, "/flash")
except OSError:
    import _inisetup
    vfs = _inisetup.setup()

# Change the working directory to /flash to be compatible with the legacy FW
os.chdir('/flash')
gc.collect()