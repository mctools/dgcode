
def perform_configuration(cmakeargs=[],
                          select_filter=None,
                          exclude_filter=None,
                          autodisable = True,
                          force_reconf=False,
                          load_all_pkgs=False,
                          prefix='',
                          quiet=False,
                          verbose=False
                      ):
    import os
    import sys
    import shutil
    import glob
    import extractenv
    import utils
    import loadpkgs
    import dirs
    import conf
    import error
    import dynpkg
    import mtime
    import pickle
    #Possible external dependencies (based solely on files in ExtDep directory):

    possible_extdeps = set(os.path.basename(f)[7:-6] for f in glob.glob(os.path.join(dirs.cmakedetectdir,'optional/ExtDep_*.cmake')))

    #Inspect package tree and load the necessary config files, given the filters:
    pl = loadpkgs.PackageLoader(dirs.pkgsearchpath,
                                possible_extdeps,
                                select_filter,
                                exclude_filter,
                                autodeps=conf.autodeps,
                                load_all=load_all_pkgs)


    volatile_misc = ['DGCODE_PROJECTS_DIR', 'DGCODE_INSTALL_DIR_RESOLVED','DGCODE_BUILD_DIR_RESOLVED', 'DGCODE_PKG_PATH', 'DGCODE_ENABLE_PROJECTS_PKG_SELECTION_FLAG'] 

    #Inspect environment via cmake (caching the result of previous runs).

    def current_reconf_environment(envdict):
        from distutils.spawn import find_executable#similar to shell "which"
        return (dict( (b,find_executable(b)) for b in set(envdict['autoreconf']['bin_list'].split(';'))),
                dict( (e,os.getenv(e)) for e in set([*envdict['autoreconf']['env_list'].split(';'),*volatile_misc]))) 

    assert dirs.blddir.exists()
    assert dirs.blddir_indicator.exists()
    envdict=None

    if not force_reconf and os.path.exists(dirs.envcache):
        #load from cache:
        envdict=utils.pkl_load(dirs.envcache)
        #check if reconf has been triggered:
        if envdict['_cmakeargs']!=cmakeargs:
            if not quiet:
                print("%sChange in cmake variables detected => reinspecting environment"%prefix)
            envdict=None
        elif envdict['_autoreconf_environment']!=current_reconf_environment(envdict):
            if not quiet:
                print("%sChange in environment detected => reinspecting via CMake"%prefix)
            envdict=None

    #Make sure any change in reported python version triggers reconf (especially important during python2->python3 transision):
    pyversionstr = str(sys.version_info)
    if envdict and envdict.get('_pyversion',None)!=pyversionstr:
        if not quiet:
            print("%sChange in python environment detected => reinspecting via CMake"%prefix)
            envdict=None

    import env
    if envdict:
        #Can reuse config:
        env.env=envdict
    else:
        #must extract the hard way via cmake:
        envdict = extractenv.extractenv(dirs.blddir,dirs.cmakedetectdir,cmakeargs=cmakeargs,quiet=quiet,verbose=verbose,prefix=prefix)
        if not envdict:
            utils.rm_f(dirs.envcache)
            error.error('Failure during cmake configuration')
        assert not '_cmakeargs' in envdict
        envdict['_cmakeargs']=cmakeargs
        assert not '_autoreconf_environment' in envdict
        envdict['_autoreconf_environment']=current_reconf_environment(envdict)
        envdict['_pyversion'] = pyversionstr
        envdict['system']['volatile'].update({ 'misc': {**dict((e,os.getenv(e)) for e in volatile_misc)} })
        utils.pkl_dump(envdict,dirs.envcache)
        #configuration fully extracted:
        env.env=envdict
        #(re)create setup file whenever extracting env via cmake:
        import create_setup_file
        create_setup_file.recreate()
        #(re)create any libs_to_symlink whenever extracting env via cmake:
        dirs.create_install_dir()
        linkdir=dirs.installdir / 'lib' / 'links'
        utils.rm_rf(linkdir)
        utils.mkdir_p(linkdir)
        for lib_to_symlink in env.env['system']['runtime']['libs_to_symlink']:
            #Symlink not only the given file, but also all other files in target
            #dir with same basename and pointing to same file (e.g. if libA.so
            #and libA.x.so both point to libA.so.x.y, then add symlinks for all
            #three names.
            lib_to_symlink=os.path.abspath(lib_to_symlink)
            bn,dn=os.path.basename(lib_to_symlink),os.path.dirname(lib_to_symlink)
            assert bn
            rp=os.path.realpath(lib_to_symlink)
            for f in glob.glob(os.path.join(dn,'%s*'%bn.split('.')[0])):
                if os.path.realpath(f)==rp:
                    os.symlink(rp,os.path.join(linkdir,os.path.basename(f)))

    #update extdep, language info and runtime files, to trigger makefile target rebuilds as necessary:
    #utils.mkdir_p(os.path.join(dirs.blddir,'extdeps'))
    utils.mkdir_p(dirs.blddir / 'extdeps')
    for extdep,info in envdict['extdeps'].items():
        utils.update_pkl_if_changed(info,dirs.blddir / 'extdeps' / extdep)

    utils.mkdir_p(dirs.blddir / 'langs')
    for lang,info in envdict['system']['langs'].items():
        utils.update_pkl_if_changed(info,dirs.blddir / 'langs' / lang)

    #Gracefully disable packages based on missing external dependencies

    missing_extdeps = set(k for k,v in envdict['extdeps'].items() if not v['present'])
    missing_needed = set(missing_extdeps).intersection(pl.active_deps_extnames())

    if missing_needed:
        if autodisable:
            disabled = pl.disable_pkgs_with_extdeps(missing_extdeps)
            if disabled and not quiet:
                print('%sDisabled %i packages as result of missing external dependencies'%(prefix,len(disabled)))
        else:
            error.error('Selected packages have missing external dependencies: "%s"'%'", "'.join(missing_needed))

    #Load old contents of global db:
    import db

    #Check if volatile parts of environment changed:
    volatile=env.env['system']['volatile']
    if db.db.get('volatile',None)!=volatile:
        db.db['volatile']=volatile
        db.db['pkg2timestamp']={}#make all pkgs "dirty"

    #For each enabled package, put it in one of the following categories:
    #   a) Timestamps of package and all parents are unchanged
    #        => Do nothing for package (all info needed by other packages will be in the global
    #                                   db and the makefile for the package is still there).
    #   b) Timestamp of package unchanged but some parent had new timestamp.
    #        => Load pkg targets from pickle file (they still need to have setup() called).
    #   c) Timestamp of package itself changed.
    #        => Create targets for package from scratch.
    #
    #As part of this, all dynamic packages must be rebuilt when appropriate.

    ts=db.db['pkg2timestamp']
    rd=db.db['pkg2reldir']

    #TODO: Implement the following for increased safety: We should first check
    #all packages if they moved reldirname or changed isdynamicpkg value. Those
    #packages all have to be nuked for safety, and if any packages are nuked,
    #all must be reconfigured (updating a symlink is not enough, as it could be
    #that the compiler reads the ultimate located of links and embeds the
    #realpath in executables). Moving packages or changing isdynamicpkg flags
    #should be rather rare, so best to be sure.

    for p in pl.enabled_pkgs_iter():
        mt=mtime.mtime_pkg(p)
        dynpkgbuilder = dynpkg.DynPkgBuilder(p) if p.isdynamicpkg else None
        if dynpkgbuilder:
            mt = max(mt,dynpkgbuilder.get_mtime_deps())
        if ts.get(p.name,None)==mt:
            p.files_changed = False
        else:
            if dynpkgbuilder:
                dynpkgbuilder.rebuild_dynamic_package(prefix)#NB: this removes any old pkgs/<pkgname> symlink in case package was just made dynamic.
            p.files_changed = True
            p.set_parent_changed_recursively()
            ts[p.name]=mt
        if rd.get(p.name,None)!=p.reldirname:
            if not p.isdynamicpkg:
                #Make sure pkg symlink is in place (refering to the package in
                #symlinked location allows users to move pkgs around). Dynamic
                #packages already updated everything during the call to
                #rebuild_dynamic_package above, but those that *used* to be
                #dynamic but are now static, might have leftover a real
                #directory with obsolete contents:
                lt=dirs.pkg_dir(p)
                if os.path.isdir(lt) and not os.path.islink(lt):
                    utils.rm_rf(lt)
                else:
                    utils.mkdir_p(os.path.dirname(lt))
                    if os.path.lexists(lt):
                        os.remove(lt)
                    os.symlink(p.dirname,lt)
            rd[p.name]=p.reldirname#ok to update immediately?

    def nuke_pkg(pkgname):
        import pipes
        conf.uninstall_package(pipes.quote(str(dirs.installdir)),pipes.quote(pkgname))
        utils.rm_rf(dirs.pkg_cache_dir(pkgname))
        utils.rm_rf(dirs.pkg_dir(pkgname))#remove link to pkg or dynamic package contents.
        nt=pipes.quote(os.path.join(dirs.blddir,'named_targets',pkgname))
        utils.system('rm -f %s %s_*'%(nt,nt))#Fixme: this is potentially
                                             #clashy... we should not use single
                                             #underscores if they are allowed in
                                             #package names (see also fixmes in
                                             #conf.py).
        db.clear_pkg(pkgname)

    import target_builder
    any_enabled=False
    for p in pl.enabled_pkgs_iter():
        any_enabled=True
        if p.files_changed:
            #recreate pkg targets from scratch
            old_parts = db.db['pkg2parts'].get(p.name,set())
            db.db['pkg2parts'][p.name]=set()
            err_txt,unclean_exception = None,None
            try:
                target_builder.create_pkg_targets(p)#this also dumps to pickle!
            except KeyboardInterrupt as e:
                err_txt = "Halted by user interrupt (CTRL-C)"
            except Exception as e:
                err_txt=str(e)
                if not err_txt:
                    err_txt='<unknown error>'
                if not isinstance(e,error.Error):
                    unclean_exception = e
            if err_txt:
                #Error! Nuke the offending package and mark ALL packages for
                #reconfig (since even the OK ones will not get makefiles updated).
                pr="\n\nERROR during configuration of package %s:\n\n  %s\n\nAborting."%(p.name,err_txt.replace('\n','\n  '))
                print(pr.replace('\n','\n%s'%prefix))
                nuke_pkg(p.name)
                db.db['pkg2timestamp']={}
                db.save_to_file()
                if unclean_exception:
                    #unknown exceptions need an ugly traceback so we can notice and try to avoid them in the future
                    error.print_traceback(unclean_exception,prefix)
                error.clean_exit(1)
            new_parts = db.db['pkg2parts'][p.name]
            disappeared_parts = old_parts.difference(new_parts)
            if disappeared_parts:
                conf.deinstall_parts(dirs.installdir,p.name,new_parts,disappeared_parts)
        elif p.any_parent_changed():
            #merely load targets from pickle!
            target_builder.create_pkg_targets_from_pickle(p)
        else:
            #nothing to be done
            pass

    if not any_enabled:
        error.error('No packages were enabled in the present configuration')

    #Global targets:
    global_targets = target_builder.create_global_targets()

    #Perform setup calls to targets:
    for p in pl.enabled_pkgs_iter():
        if hasattr(p,'targets'):
            for t in p.targets:
                assert not t.isglobal
                t.setup(pl.name2pkg[t.pkgname])

    for t in global_targets:
        assert t.isglobal
        t.setup(None)#has no associated pkg

    #Validate and collect:
    all_targets=[]
    for p in pl.enabled_pkgs_iter():
        if hasattr(p,'targets'):
            for t in p.targets:
                t.validate()
            all_targets += p.targets

    for t in global_targets:
        t.validate()
    all_targets+=global_targets

    #setup makefiles:
    utils.mkdir_p(dirs.makefiledir)
    import makefile

    #package makefiles:

    enabled_pkgnames=[]
    for p in pl.enabled_pkgs_iter():
        enabled_pkgnames+=[p.name]
        if hasattr(p,'targets'):#no targets means reuse old makefile
            makefile.write_pkg(p)

    #the main makefile:
    makefile.write_main(global_targets,enabled_pkgnames)

    enabled_pkgnames=set(enabled_pkgnames)
    if 'enabled_pkgnames' in db.db:
        import pipes
        #Test if any pkgs got disabled compared to last time and remove it from install area:
        for disappeared_pkgname in db.db['enabled_pkgnames'].difference(enabled_pkgnames):
            if not quiet:
                print("%sUninstalling package %s"%(prefix,disappeared_pkgname))
            nuke_pkg(disappeared_pkgname)
    db.db['enabled_pkgnames'] = enabled_pkgnames

    #Save new state:
    db.save_to_file()

    dgtestfile=dirs.blddir / 'dgtests'
    if not os.path.exists(dgtestfile):
        fh=open(dgtestfile,'w')
        for l in open(os.path.join(dirs.sysdir,'dgtests_template')):
            if l=='%%INCLUDED_FUNCTIONS%%\n':
                import inspect
                fh.write(''.join(inspect.getsourcelines(conf.runnable_is_test)[0]))
                fh.write(''.join(inspect.getsourcelines(utils.mkdir_p)[0]))
                fh.write(''.join(inspect.getsourcelines(utils.system)[0]))
            else:
                fh.write(l)
        fh.close()

    dir_env_vars = {
      'includedir': 'ESS_INCLUDE_DIR',
      'installprefix': 'ESS_INSTALL_PREFIX',
      'testrefdir': 'ESS_TESTREF_DIR',
      'datadir': 'ESS_DATA_DIR',
      'libdir': 'ESS_LIB_DIR'
    }
    dirdict = { key: os.environ.get(value) for (key, value) in dir_env_vars.items() if os.environ.get(value, None)}

    dir_names = ('sysdir','fmwkdir','projdir','pkgsearchpath','blddir','installdir','testdir','envcache')
    dirdict.update(dict(((d, getattr(dirs, d)) for d in dir_names)))

    #Update dynamic python module with information, if needed:
    cfgfile=dirs.blddir / 'cfg.py'
    dinfo = dict((p.name,p.info_as_dict()) for p in pl.pkgs)
    content='"""File automatically generated by dgbuild"""' 
    content+=f"""
import pickle
pkgs = pickle.loads({pickle.dumps(dinfo)})
import pathlib as _pl
class Dirs: 
"""
    for k,v in sorted(dirdict.items()): 
      val = (f"_pl.Path({repr(str(v))})" if not isinstance(v,list)
             else '['+','.join([f"_pl.Path({repr(str(el))})" for el in v])+']')
      content+=f"""
  @property
  def {k}(self):
    return {val} 
"""
    content+="""
dirs = Dirs()
if __name__=="__main__":
  import pprint as pp
  pp.pprint(pkgs) 
  pp.pprint({ p:getattr(dirs,p) for p in dir(dirs) if not p.startswith('_')})
""" 
    #TODO: Don't always just overwrite the config file (the pickled state is
    #indeterministic, so can't quickly tell from the serialised data if anything
    #changed or not):
    with open(cfgfile,'wt') as f:
        f.write(content)

    return pl
