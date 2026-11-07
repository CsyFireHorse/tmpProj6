import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '../../build/lib.win-amd64-cpython-310'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../build/lib.win-amd64-cpython-312'))
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from typing import List
import zfbWrap

   
class AxFbAttrWrap:
    def __init__(self, impl) -> None:
        self.impl = impl
    def get(self, attr): 
        return zfbWrap.axfb_attr_get(self.impl)
    def setAttr(self, attr, v):
        return zfbWrap.axfb_attr_set(self.impl, attr, v)
    @staticmethod
    def create():
        return AxFbAttrWrap(zfbWrap.axfb_attr_create())
    
class AxfbCmdMgr:
    def __init__(self) -> None:
        self.cfg = zfbWrap.axfb_cmd_config_create()
        self.mgr = zfbWrap.axfb_cmd_manager_create()
    def loadConfigFromFile(self, pth: str):
        ret = zfbWrap.load_axfb_cmd_config_from_json(self.cfg, pth)
        if ret:
            print("load failed\n")
            return
        zfbWrap.axfb_set_config(self.mgr, self.cfg)
        
    def run(self):
        zfbWrap.axfb_run(self.mgr)
    def runCycle(self):
        zfbWrap.axfb_run_cycle(self.mgr)
    def getAxis(self, idx: int):
        return AxFbAttrWrap(zfbWrap.axfb_get_axis_ptr(self.mgr, idx))
    def getFb(self, idx: int):
        return AxFbAttrWrap(zfbWrap.axfb_get_fb_ptr(self.mgr, idx))
    def __del__(self):
        zfbWrap.axfb_cmd_config_destroy(self.cfg)
        zfbWrap.axfb_cmd_manager_destroy(self.mgr)
    
def AxfbCmdMgr_demo(pth):
    mgr = AxfbCmdMgr()
    mgr.loadConfigFromFile(pth)
    ax = mgr.getAxis(1)
    fb = mgr.getFb(2)
    for i in range(100):
        if i<10:
            ax.setAttr(zfbWrap.AX_ATTR_DIENCODERPOSITION, i*29/3)
        else:
            fb.setAttr(zfbWrap.FB_ATTR_EXECUTE, 1)
        mgr.runCycle()
    # mgr.run()
    
if __name__ == "__main__":
    pth = "../fb/demo/motbuf_cmds.json"
    pth = "demo/motbuf_cmds.json"
    AxfbCmdMgr_demo(pth)
