import os
from . import dirs
from . import col
from . import env
from . import utils
import pipes
import shlex

def _write_targets(fh,targets):
    cmdpat='\t${P}%s\n'
    for t in targets:
        tn=t.name
        if not '/' in t.name: tn='${TRG}/%s'%tn
        l=['%s:'%tn]
        for dn in t.deps:
            l+=[dn if '/' in dn else '${TRG}/%s'%dn]
        fh.write('%s\n'%' '.join(l))
        for c in t.code:
            fh.write(cmdpat%c)
        if not '/' in t.name:
            fh.write(cmdpat%('touch %s'%tn))
        if False and not t.contains_message:
            fh.write('\t${P}echo Target %s done\n\n'%t.name)
        else:
            fh.write('\n')

def ldflags_to_rpath_dirs(ldflags):
    res=[]
    if not ldflags:
        return res
    #NB: CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES does not always include all
    #relevant dirs, so we hardcode a list here:
    ignore=set(['/lib','/lib32','/lib64','/usr/lib','/usr/lib32','/usr/lib64'])
    for e in utils.shlex_split(ldflags):
        if e.startswith('-L'):
            e=e[2:]
        if e and os.path.exists(e) and (e.endswith('.so') or e.endswith('.dylib')):
            e=os.path.dirname(e)
        if e and e not in ignore and os.path.isdir(e):
            e=os.path.abspath(os.path.realpath(e))
            if not e in res:
                res += [e]
    return res

def write_main(global_targets,enabled_pkgnames):
    #the main makefile

    fh = open(dirs.makefiledir / 'Makefile','w')

    #global variables:
    fh.write('#Autogenerated file -- changes will be lost\n')
    fh.write('VERBOSE = 0\n\n')#can be overridden from cmd line
    fh.write('ifeq ($(VERBOSE), 1)\n')
    fh.write('  P := \n')#dont silence command printing
    fh.write('else\n')
    fh.write('  P := @\n')#silence command printing
    fh.write('endif\n\n')
    fh.write('MDIR := %s\n'%dirs.makefiledir)
    fh.write('INST := %s\n'%dirs.installdir)
    fh.write('PKG := %s\n'%dirs.pkgdirbase)
    #fh.write('SRC := %s\n'%dirs.codedir)
    fh.write('BLD := %s\n'%dirs.blddir)
    fh.write('SYS := %s\n'%dirs.sysdir)
    fh.write('INC := ${INST}/%s\n'%dirs.incdirname)
    fh.write('TRG := ${BLD}/named_targets\n\n')
    fh.write('EXT := ${BLD}/extdeps\n\n')
    fh.write('LANG := ${BLD}/langs\n\n')
    fh.write('CHEAT_RUN0 := $(shell mkdir -p ${BLD}/named_targets)\n\n')

    for cname in [c for c in dir(col) if c.startswith('bldmsg_')]:
        fh.write('COL_%s := "%s"\n'%(cname.upper(),eval('col.%s'%cname)))
    fh.write('COL_END := "%s"\n\n'%col.end)

    extdeps=env.env['extdeps']
    for extdep,info in extdeps.items():
        if info['present']:
            fh.write('CFLAGS_CXX_%s := %s\n'%(extdep,info['cflags_cxx']))
            fh.write('CFLAGS_C_%s := %s\n'%(extdep,info['cflags_c']))
            ldflags=info['ldflags']
            fh.write('LDFLAGS_%s := %s\n'%(extdep,ldflags))
            rpathdirs=[]
            if ldflags:
                rpathdirs = ldflags_to_rpath_dirs(info['ldflags'])
            for lang,info in env.env['system']['langs'].items():
                if info:#info is only available for available languages:
                    fh.write('LDFLAGS_%s_EXE_%s := %s\n'%(extdep,lang,' '.join([info['rpath_flag_exe']%pipes.quote(e) for e in rpathdirs])))
                    fh.write('LDFLAGS_%s_LIB_%s := %s\n'%(extdep,lang,' '.join([info['rpath_flag_lib']%pipes.quote(e) for e in rpathdirs])))

    fh.write('\n')

    use_sysboostpy = env.env['system']['general']['sysboostpython_use']
    if use_sysboostpy:
        _c = env.env['system']['general']['sysboostpython_cflags'].strip()
        _ = shlex.quote( str( (dirs.blddir/'sysboostinc').absolute().resolve() ) )
        _c += ' -DDGCODE_USESYSBOOSTPYTHON -I%s -isystem%s'%(_,_)
        _l = env.env['system']['general']['sysboostpython_linkflags'].strip()
    else:
        _c, _l = '',''
        if dirs.sysinc_shippedboost:
            _c = ' -I%s -isystem%s'%(dirs.sysinc_shippedboost,dirs.sysinc_shippedboost)
    fh.write('DGBOOSTCFLAGS := %s\n'%_c)
    def finalise_boost_ldflags( flags, shlib ):
        ra = utils.rpath_appender(lang='cxx',shlib=shlib)
        cxxldflags = env.env['system']['langs']['cxx']['ldflags']
        res = []
        for f in ra.apply_to_flags( shlex.split(flags) ):
            if not f in cxxldflags:
                res.append( f )
        return ' '.join( shlex.quote(f) for f in res )

    fh.write('DGBOOSTLDFLAGS_EXE := %s\n'%finalise_boost_ldflags(_l,shlib=False) )
    fh.write('DGBOOSTLDFLAGS_LIB := %s\n'%finalise_boost_ldflags(_l,shlib=True) )

    for lang,info in env.env['system']['langs'].items():
        if info:#info is only available for available languages:
            _c, _l = info['cflags'], info['ldflags']
            if lang=='cxx':
                _c += ' ${DGBOOSTCFLAGS}'
                #_l += ' ${DGBOOSTLDFLAGS}'
            fh.write('CFLAGSLANG_%s := %s\n'%(lang,_c))
            fh.write('LDFLAGSLANG_%s := %s\n'%(lang,_l))
            fh.write('LDFLAGSPREPENDLANG_%s := %s\n'%(lang,info['ldflags_prepend']))

    fh.write('\n')

    fh.write('ifeq ($(VERBOSE), -1)\n')
    fh.write('else\n')
    fh.write('  ifeq ($(COL_END), "")\n')
    fh.write('    $(info Build started)\n')
    fh.write('  else\n')
    fh.write('    $(info [94mBuild started[0m)\n')
    fh.write('  endif\n')
    fh.write('endif\n')

    #disable implicit rules
    fh.write('.SUFFIXES:\n\n')

    #Default target gathers up all pkg targets:
    fh.write('.PHONY: all\n\n')

    fh.write('%s\n\t@if [ ${VERBOSE} -ge 0 ]; then echo "%sAll done%s"; fi\n\n'%(' '.join(['all:']+['${TRG}/%s'%e for e in enabled_pkgnames]+['${TRG}/%s'%t.name for t in global_targets]),col.bldcol('global'),col.bldend))

    #Include package makefiles:
    for p in enabled_pkgnames:
        fh.write('include ${MDIR}/Makefile_%s.make\n'%p)

    fh.write('\n')
    #all other targets:
    _write_targets(fh,global_targets)

def write_pkg(pkg):
    assert pkg.targets

    fh = open(dirs.makefiledir.joinpath('Makefile_%s.make'%pkg.name),'w')


    #disable implicit rules
    fh.write('.SUFFIXES:\n\n')

    _write_targets(fh,pkg.targets)

