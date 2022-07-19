from __future__ import print_function
from __future__ import absolute_import
__metaclass__ = type#py2 backwards compatibility

import os
import sys
import shutil
import glob

import conf
import col
import utils
import dirs
import error
import dirs
import mtime

_dynpreamble="""from __future__ import print_function,absolute_import,division
__metaclass__ = type#py2 backwards compatibility
import os,sys,shutil
from utils import mkdir_p as __mkdir_p
__targetblddir=['/non/existing/dir']
def _set_targetblddir(d):
    __targetblddir[0] = d

def __preptarget(filename):
    if not len(filename.split(os.sep))==2 or filename.startswith('.'):
        raise RuntimeError('Illegal filename (expected form is <subdir>/<filename>): "%%s"'%%filename)
    assert __targetblddir[0] != '/non/existing/dir'
    fn = os.path.join(__targetblddir[0],filename)
    if os.path.exists(fn):
        raise RuntimeError('Creating the same file more than once: "%%s"'%%filename)
    __mkdir_p(os.path.dirname(fn))
    return fn

def create_file(filename,contents,mode='wt'):
    with open(__preptarget(filename),mode) as fh:
        fh.write(contents)

def link_file(src_abspath,target_filename):
    if not os.path.exists(src_abspath):
        raise RuntimeError('Linking to non-existing file: "%%s"'%%src_abspath)
    os.symlink(src_abspath,__preptarget(target_filename))

#Code from %s:\n###################################################################\n"""

class DynPkgBuilder:

    def __init__(self,pkg):
        assert pkg.isdynamicpkg
        assert pkg.dynamicpkgcode
        self.pkg = pkg
        self._dynmoddir = dirs.pkg_cache_dir(pkg,'dynpkgmod')
        self._usrdir = dirs.pkg_cache_dir(pkg,'dynpkgusr')#where the user code will create content
        self._dynmodname=os.path.join(self._dynmoddir,'%s.py'%pkg.name)
        self._pkgcfgfile = os.path.join(pkg.dirname,conf.package_cfg_file)
        self._fct_getdeppaths = None
        self._fct_blddynpkg = None
        self._safety_deployed = False

        #Check that we only have the pkg.info files in the static dirs of dynamic packages:
        for ff in os.listdir(pkg.dirname):
            if not conf.ignore_file(ff) and ff!=conf.package_cfg_file:
                isdir=os.path.isdir(os.path.join(pkg.dirname,ff))
                if isdir and ff.startswith('external'):
                    continue
                error.error('Forbidden %s "%s" found in dynamic package %s (only the %s file and "external.*/" directories are allowed in such packages)'%('directory' if isdir else 'file',
                                                                                                                                                           ff,pkg.name,conf.package_cfg_file))

    def _write_modfile(self):
        utils.mkdir_p(self._dynmoddir)
        with open(self._dynmodname,'wt') as dmod:
            dmod.write(_dynpreamble%self._pkgcfgfile)
            dmod.write(self.pkg.dynamicpkgcode)

    def _ensure_updated_modfile(self):
        if not os.path.exists(self._dynmodname) or mtime.mtime(self._dynmodname) < mtime.mtime(self._pkgcfgfile):
            self._write_modfile()

    def _loadmod(self):
        assert self._fct_getdeppaths is None and self._fct_blddynpkg is None, "Reload of custom modules not supported"
        self._ensure_updated_modfile()
        #sanity check that there are not cmdline arguments which might be inspected by dynamic code:
        assert len(sys.argv)==1
        try:
            thedynmod = utils.import_modatpath(self._dynmodname)
        except Exception as e:
            error.print_traceback(e)
            if isinstance(e,SyntaxError):
                error.error('SyntaxError in dynamic_builder code in %s/%s'%(self.pkg.name,conf.package_cfg_file))
            else:
                error.error('Exception thrown from dynamic_builder code in %s/%s'%(self.pkg.name,conf.package_cfg_file))
        def getfct(fn):
            fct=getattr(thedynmod,fn,None)
            if not fct or not hasattr(fct,'__call__'):
                error.error('missing %s function in dynamic_builder code in %s/%s'%(fn,self.pkg.name,conf.package_cfg_file))
            return fct
        self._fct_getdeppaths = getfct('dynamic_package_deppaths')
        #for build_dynamic_package, ensure it always gets a call to _set_targetblddir first, so utility functions work properly:
        fct_blddynpkg_raw = getfct('build_dynamic_package')
        fct_settargetblddir_raw = getfct('_set_targetblddir')
        self._fct_blddynpkg = lambda s,t : [fct_settargetblddir_raw(t),fct_blddynpkg_raw(s)][1]

    def get_deppaths(self):
        if self._fct_getdeppaths is None:
            self._loadmod()
        deppaths = self._fct_getdeppaths(self.pkg.dirname)
        if not isinstance(deppaths,list) or not deppaths:
            error.error( 'dynamic_package_deppaths function must return non-empty list of paths whose timestamps will be monitored in %s' % self._pkgcfgfile )
        return deppaths

    def get_mtime_deps(self):
        return mtime.mtime_pathlist(self.get_deppaths())

    def _possible_unsafe_link_timestamp_detected(self):
        if self._safety_deployed:
            return
        #A file (or link) was replaced by a symlink, which could point to
        #something with an old timestamp, not triggering a rebuild like it
        #should. For safety, we ensure rebuild of all object files in this
        #package, and of all named targets as well - ensuring rebuilds of
        #targets in downstream packages:
        for f in glob.glob(dirs.blddir.joinpath('named_targets','%s_*'%self.pkg.name)):
            utils.rm_f(f)
        for f in glob.glob(dirs.pkg_cache_dir(self.pkg,'objs/*/*_deps.pkl')):
            utils.touch(f)
        self._safety_deployed=True

    def rebuild_dynamic_package(self,prefix=''):

        if self._fct_blddynpkg is None:
            self._loadmod()
        #prepare empty directory in which user code will create dynamic content:
        ud=self._usrdir
        utils.rm_rf(ud)
        utils.mkdir_p(ud)

        #Let custom code from pkg.info file build the dynamic content:
        print('%s%sCommencing creation of dynamic package: %s%s'%(prefix,col.bldmsg_pkg,self.pkg.name,col.end))
        self._fct_blddynpkg(self.pkg.dirname,ud)

        #Explicit error against adding package cfg files:
        if os.path.exists(os.path.join(ud,conf.package_cfg_file)):
            error.error('Dynamic package builder created %s - but such a file was already present in the'
                        +' package and should not be added dynamically.'%conf.package_cfg_file)

        #Create list of files created by user code - with lots of sanity checking.
        filelist={}
        for subdir in sorted(os.listdir(ud)):
            if conf.ignore_file(subdir):
                error.error('Dynamic package builder created directory which will be ignored: %s'%subdir)
            abssubdir=os.path.join(ud,subdir)
            if not os.path.isdir(abssubdir):
                error.error('Dynamic package builder created forbidden file (all files must be placed in subdirs): %s'%subdir)
            fl=[]
            for f in sorted(os.listdir(abssubdir)):
                if conf.ignore_file(f):
                    error.error('Dynamic package builder created file which will be ignored: %s'%os.path.join(subdir,f))
                absf=os.path.join(abssubdir,f)
                if os.path.isdir(absf):
                    error.error('Dynamic package builder created forbidden directory (no subdirs in subdirs): %s'%os.path.join(subdir,f))
                if not os.path.exists(absf):
                    error.error('Dynamic package builder created invalid file (broken link?): %s'%os.path.join(subdir,f))
                fl+=[f]#ok file
            if not fl:
                error.error('Dynamic package builder created empty directory: %s'%subdir)
            filelist[subdir]=fl
        if not filelist:
            error.error('Dynamic package builder created no files!')

        #Prepare destination of files (extra careful not to affect timestamp when not needed):
        pd = dirs.pkg_dir(self.pkg)
        if not os.path.exists(pd):
            utils.mkdir_p(pd)

        #Remove any previously created files which have now disappeared or are now broken symlinks:
        for sd in os.listdir(pd):
            fl=filelist.get(sd,None)
            abssd=os.path.join(pd,sd)
            #NB: os.path.exists(...) will fail for broken symlinks
            if not fl or not os.path.exists(abssd):
                utils.rm_rf(os.path.join(pd,sd))
            else:
                for f in os.listdir(abssd):
                    absf=os.path.join(pd,sd,f)
                    if not f in fl or not os.path.exists(absf):
                        utils.rm_f(absf)

        #create/copy over any non-existing files and dirs:
        def stat_fingerprint(f):
            #permissions and size must match
            s=os.stat(f)
        def same_contents(f1,f2):
            with open(f1,'rb') as fh1, open(f2,'rb') as fh2:
                return ( fh1.read() == fh2.read() )

        for subdir,fl in sorted(filelist.items()):
            target_sd=os.path.join(pd,subdir)
            if not os.path.exists(target_sd):
                utils.mkdir_p(target_sd)
            for f in fl:
                src_f = os.path.join(ud,subdir,f)
                src_islink = os.path.islink(src_f)
                src_realpath = os.path.realpath(src_f)
                if src_islink and src_realpath.startswith(str(dirs.blddir)):
                    #we could in principle support this, but easier not to.
                    error.error('Dynamic package builder created file which is a symlink to a temporary file: %s'%os.path.join(subdir,f))
                assert os.path.exists(src_f)
                target_f = os.path.join(pd,subdir,f)
                if os.path.islink(target_f):
                    if src_islink and os.path.realpath(target_f)==src_realpath:
                        continue#link already pointing at correct location
                    else:
                        #should no longer be a link or is incorrect link, so remove:
                        if src_islink:
                            self._possible_unsafe_link_timestamp_detected()
                        utils.rm_f(target_f)

                assert not os.path.islink(target_f)
                if os.path.exists(target_f):
                    #existing non-link file found
                    if src_islink:
                        self._possible_unsafe_link_timestamp_detected()
                        utils.rm_f(target_f)
                        os.symlink(src_realpath,target_f)
                    else:
                        #Only re-copy file if either contents or attributes changed:
                        if stat_fingerprint(src_f) == stat_fingerprint(target_f) and same_contents(src_f,target_f):
                            continue#File already has correct attributes + contents, no need to disturb its timestamp!
                        utils.rm_f(target_f)
                        shutil.copy2(src_f,target_f)
                else:
                    if src_islink:
                        os.symlink(src_realpath,target_f)
                    else:
                        shutil.copy2(src_f,target_f)

        #Finally link over the package config file if needed:
        pdcfg=os.path.join(pd,conf.package_cfg_file)
        if not os.path.exists(pdcfg) or os.path.realpath(pdcfg) != os.path.realpath(self._pkgcfgfile):
            utils.rm_f(pdcfg)
            os.symlink(self._pkgcfgfile,pdcfg)

        #cleanup temporary files:
        utils.rm_rf(self._usrdir)

        #TODO: --verbose should put some output here and from the specific actions above!!
        #TODO: Count total + updated files, so can say "updated i/n files (and removed k files)"
        print('%s%sCreated dynamic package: %s%s'%(prefix,col.bldmsg_pkg,self.pkg.name,col.end))
