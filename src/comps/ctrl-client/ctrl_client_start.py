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

if 'ctrl_config' not in globals().keys():
    from ctrl_config import CtrlConfig
    from ctrl import Ctrl

    ctrl_config = CtrlConfig().read_config()

if (not ctrl_config.get('ctrl_autostart', True)) and ctrl_config.get('cfg_msg') is not None:
    print(ctrl_config.get('cfg_msg'))
    print("Not starting CTRL as auto-start is disabled")

else:
    # Load CTRL if it is not already loaded
    if 'ctrl' not in globals().keys():
        print("-- start ctrl client")
        try:
            ctrl = Ctrl(ctrl_config, ctrl_config.get('cfg_msg') is None, True)
        except Exception:
            from ctrl_sb import CtrlSafeBoot
            ctrl = CtrlSafeBoot()
