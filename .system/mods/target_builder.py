from . import target_base
from . import col
import os
import re
from . import dirs
from . import utils
from . import error
from . import conf

patterns=[]
dirtypes=set()
dirtype_info = {}
def register_pattern(pattern,target_factory):
    global patterns,dirtypes,dirtypes_info
    patterns+=[pattern]
    dt=pattern[0:4]
    dirtype_info[dt] = (re.compile('^'+pattern+'$').match,target_factory)
    assert len(dirtype_info)==len(patterns),"patterns not uniquely defined by first 4 chars"

for p,f in conf.target_factories_for_patterns():
    register_pattern(p,f)
nonpattern_tfacts=conf.target_factories()

def apply_patterns(pkg):
    targets=[]
    dn=dirs.pkg_dir(pkg)
    dirtypes=set()
    for d in os.listdir(dn):
        dirtype=d[0:4]
        nfo = dirtype_info.get(dirtype,None)
        dd=os.path.join(dn,d)
        if nfo and nfo[0](d) and os.path.isdir(dd):
            l=nfo[1](pkg,d)#call factory
            #subdirs matching a target-generating pattern can not have subdirs themselves. This seems like a good place to check for it:
            for sf in os.listdir(dd):
                if sf=='.ipynb_checkpoints':
                    continue#ignore these as a workaround (although we should guard against them being committed)
                sf=os.path.join(dd,sf)
                if os.path.isdir(sf):
                    if os.path.basename(sf).startswith('__pycache__'):
                        continue
                    error.error('Sub-directories are not allowed in %s/%s/'%(pkg.name,d))
            if l:
                dirtypes.add(dirtype)
                targets += l
    return targets,dirtypes

def create_pkg_targets(pkg):
    assert not hasattr(pkg,'targets')
    #create targets for package
    targets=[]

    #find subdirs matching patterns and apply their associated target factory:
    pt,dt=apply_patterns(pkg)
    targets += pt

    for tf in nonpattern_tfacts:
        targets+=tf(pkg,dt)

    #Packages always have special done target which depends on all the other targets:
    targets += [target_base.TargetPkgDone(pkg,targets)]

    pkg.targets = targets

    d=dirs.pkg_cache_dir(pkg)
    utils.mkdir_p(d)
    utils.pkl_dump(targets,os.path.join(d,'targets.pkl'))

def create_pkg_targets_from_pickle(pkg):
    assert not hasattr(pkg,'targets')
    pkg.targets = utils.pkl_load(dirs.pkg_cache_dir(pkg,'targets.pkl'))

class TargetGlobalSysModules(target_base.Target):
    isglobal=True
    contains_message=True
    def __init__(self):
        self.name='global__sysmods'
        self.pkgname = None
        self.deps = []
        self.code = ['@if [ ${VERBOSE} -ge 0 ]; then echo "%sInstalling global system modules%s"; fi'%(col.bldcol('global'),col.bldend),
                     'mkdir -p ${INST}/python/simplebuild',
                     'touch ${INST}/python/simplebuild/__init__.py'
                     ]
        self.deps += [ '${BLD}/cfg.py' ]
        self.code += ['cp -f ${BLD}/cfg.py ${INST}/python/simplebuild/cfg.py']#TODO: Add more info in cfg.py.

class TargetGlobalScripts(target_base.Target): # DGBUILD-NO-EXPORT
    isglobal=True # DGBUILD-NO-EXPORT
    contains_message=True # DGBUILD-NO-EXPORT
    def __init__(self): # DGBUILD-NO-EXPORT
        self.name='global__scripts' # DGBUILD-NO-EXPORT
        self.pkgname=None # DGBUILD-NO-EXPORT
        self.deps=['${BLD}/dgtests','${SYS}/dginstall','${SYS}/dgenv'] # DGBUILD-NO-EXPORT
        self.code = ['@if [ ${VERBOSE} -ge 0 ]; then echo "%sInstalling global scripts: dgtests dginstall dgenv%s"; fi'%(col.bldcol('global'),col.bldend), # DGBUILD-NO-EXPORT
                     'mkdir -p ${INST}/sysbin', # DGBUILD-NO-EXPORT
                     'cp -f ${BLD}/dgtests ${INST}/sysbin/dgtests && chmod +x ${INST}/sysbin/dgtests', # DGBUILD-NO-EXPORT
                     'cp -f ${SYS}/dginstall ${INST}/sysbin/dginstall && chmod +x ${INST}/sysbin/dginstall', # DGBUILD-NO-EXPORT
                     'cp -f ${SYS}/dgenv ${INST}/sysbin/dgenv && chmod +x ${INST}/sysbin/dgenv'] # DGBUILD-NO-EXPORT
# DGBUILD-NO-EXPORT
class TargetSetupScript(target_base.Target): # DGBUILD-NO-EXPORT
    isglobal=True # DGBUILD-NO-EXPORT
    contains_message=True # DGBUILD-NO-EXPORT
    def __init__(self): # DGBUILD-NO-EXPORT
        self.name='global__setupscript' # DGBUILD-NO-EXPORT
        self.pkgname=None # DGBUILD-NO-EXPORT
        self.deps=['${BLD}/setup.sh'] # DGBUILD-NO-EXPORT
        self.code = ['@if [ ${VERBOSE} -ge 0 ]; then echo "%sInstalling global script: setup.sh%s"; fi'%(col.bldcol('global'),col.bldend), # DGBUILD-NO-EXPORT
                     'mkdir -p ${INST}', # DGBUILD-NO-EXPORT
                     'cp -f ${BLD}/setup.sh ${INST}/setup.sh'] # DGBUILD-NO-EXPORT

def create_global_targets():
    return [
        TargetGlobalScripts(), # DGBUILD-NO-EXPORT
        TargetGlobalSysModules(),
        TargetSetupScript() # DGBUILD-NO-EXPORT
    ]
