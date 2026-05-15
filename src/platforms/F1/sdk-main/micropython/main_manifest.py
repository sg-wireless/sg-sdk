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

freeze("$(PORT_DIR)/modules", "apa106.py")
freeze("$(PORT_DIR)/modules", "flashbdev.py")
freeze("$(PORT_DIR)/modules", "espnow.py")
freeze("$(PORT_DIR)/modules", "machine.py")
include("$(MPY_DIR)/extmod/asyncio")
freeze(".", "_boot.py")
freeze(".", "_inisetup.py")

# Require some micropython-lib modules.
require("dht")
require("ds18x20")
require("mip")
require("neopixel")
require("ntptime")
require("onewire")
require("umqtt.robust")
require("umqtt.simple")
require("upysh")
require("urequests")
require("webrepl")

require("tarfile")
require("gzip")
require("unittest")

require("cbor2")


# Useful networking-related packages.
require("bundle-networking")

# Require some micropython-lib modules.
require("aioespnow")
