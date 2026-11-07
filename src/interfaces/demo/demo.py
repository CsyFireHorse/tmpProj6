import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '../../build/lib.win-amd64-cpython-310'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../build/lib.win-amd64-cpython-312'))
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from typing import List
import zfbWrap

def demo(pth):
    cfg = zfbWrap.axfb_cmd_config_create()
    
    ret = zfbWrap.load_axfb_cmd_config_from_json(cfg, pth)
    if ret:
        print("load failed\n")
        return
    
    mgr = zfbWrap.axfb_cmd_manager_create()
    zfbWrap.axfb_set_config(mgr, cfg)
    zfbWrap.axfb_run(mgr)
    zfbWrap.axfb_cmd_config_destroy(cfg)
    zfbWrap.axfb_cmd_manager_destroy(mgr)
    
if __name__ == "__main__":
    pth = "../fb/demo/motbuf_cmds.json"
    pth = "demo/motbuf_cmds.json"
    demo(pth)

