import uos
from flashbdev import bdev


def check_bootsec():
    buf = bytearray(bdev.ioctl(5, 0))  # 5 is SEC_SIZE
    bdev.readblocks(0, buf)
    empty = True
    for b in buf:
        if b != 0xFF:
            empty = False
            break
    if empty:
        return True
    fs_corrupted()


def fs_corrupted():
    import time

    while 1:
        print(
            """\
The filesystem appears to be corrupted. If you had important data there, you
may want to make a flash snapshot to try to recover it. Otherwise, perform
factory reprogramming of MicroPython firmware (completely erase flash, followed
by firmware programming).
"""
        )
        time.sleep(3)


def setup():
    check_bootsec()
    print("Performing initial setup, creating an Lfs2 file system on /flash")
    uos.VfsLfs2.mkfs(bdev)
    vfs = uos.VfsLfs2(bdev)
    # Mount to /flash
    uos.mount(vfs, "/flash")
    # Create the default folders
    vfs.mkdir("cert")
    #vfs.mkdir("sys")
    vfs.mkdir("lib")
    with open("/flash/boot.py", "w") as f:
        f.write(
            "# This file is executed on every boot (including wake-boot from deepsleep)"
        )
    with open("/flash/main.py", "w") as f:
        f.write(
            "# This file is executed on every boot (including wake-boot from deepsleep)"
        )

    return vfs