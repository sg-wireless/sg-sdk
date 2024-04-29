
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
        ctrl = Ctrl(ctrl_config, ctrl_config.get('cfg_msg') is None, True)
