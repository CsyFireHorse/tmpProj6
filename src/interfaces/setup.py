#!/usr/bin/env python
'''
Author: ChenSy
LastEditors: ChenSy
Date: 2024-05-08 10:32:04
LastEditTime: 2025-07-22 15:09:45
Description: Modify here please
'''
from distutils.core import setup, Extension
import glob


src = ['base/source/*.cpp', 'vel/source/*.cpp', 'traj/source/*.cpp', 'fb/source/misc/*.cpp', 'fb/source/fb/*.cpp', 'fb/source/axfb/*.cpp', 'fb/source/axis/*.cpp', 'mcapi/source/*.cpp', 'mcapi/source/axfb/*.cpp', 'mcapi/source/cam/*.cpp']
src = [ '../'+s for s in src]
gbs = ['zfb_wrap.cxx']
for s in src: gbs += glob.glob(s) 

includes = ['common', 'base', 'base/include', 'vel',  'vel/include',  'traj',  'traj/include', 
'fb/', 'fb/include', 'fb/include/misc',  'fb/include/fb', 'fb/include/axis', 'fb/include/axfb',  'fb/include/impl', 'interfaces/','mcapi/', 'mcapi/include',
"thirdlib"
]
includes = [ '../'+s for s in includes] + ['..']
example_module = Extension('_zfbWrap',
       sources=gbs,
       include_dirs=includes,
)

setup (name = 'zfbWrap',
       version = '0.1.0',
       author      = "Chen Sy",
       description = """motion simulation example""",
       ext_modules = [example_module],
       py_modules = ["zfbWrap"],
)


# swig -c++ -python interfaces/zfb.i
# vsvars64.bat
# python setup.py build