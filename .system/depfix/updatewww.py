#!/usr/bin/env python3
import sys,os,pathlib,shlex

thisdir = pathlib.Path(__file__).parent.absolute()
compilecmd=thisdir.joinpath('compile.py')
assert compilecmd.exists()

cmd='%s ALWAYS'%shlex.quote(str(compilecmd))
print("Running compilation cmd: %s"%cmd)
ec=os.system(cmd)
assert ec==0,"Compilation failed!!"
dgdepfixer_file = thisdir.joinpath('bld/bin/dgdepfixer')
assert dgdepfixer_file.exists()
cmd='scp %s %s'%(shlex.quote(str(dgdepfixer_file)),
                 shlex.quote('tkittel@lxplus.cern.ch:www/depfix/'))
print("Uploading with command: %s"%cmd)
ec=os.system(cmd)
assert ec==0,"SCP failed!!"
