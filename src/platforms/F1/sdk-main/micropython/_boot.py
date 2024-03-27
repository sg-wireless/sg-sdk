import gc
import os
import uos
from flashbdev import bdev

def setup():
    print("Performing initial setup, creating an Lfs2 file system on /flash")
    uos.VfsLfs2.mkfs(bdev)
    vfs = uos.VfsLfs2(bdev)
    # Mount to /flash
    uos.mount(vfs, "/flash")
    # Create the default folders
    vfs.mkdir("cert")
    #vfs.mkdir("sys")
    vfs.mkdir("lib")
    return vfs

try:
    from flashbdev import bdev
    if bdev:
        os.mount(bdev, "/flash")
except OSError:
    vfs = setup()

# Change the working directory to /flash to be compatible with the legacy FW
os.chdir('/flash')
gc.collect()
