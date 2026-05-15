# ---------------------------------------------------------------------------- #
# This file is part of the MicroPython project, http://micropython.org/
# Copyright (c) 2016 Damien P. George
# Modified by SG Wireless - Copyright (c) 2023-2026 SG Wireless - All Rights
# Reserved
#
# MIT License - see above permission notice.
# ---------------------------------------------------------------------------- #

import gc
import os

try:
    from flashbdev import bdev
    if bdev:
        os.mount(bdev, "/")
except OSError:
    import _inisetup
    vfs = _inisetup.setup()

gc.collect()