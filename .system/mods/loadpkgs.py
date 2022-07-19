from __future__ import print_function
from __future__ import division
__metaclass__ = type#py2 backwards compatibility
import os,sys,dirs
import utils,langs,conf,error
join=os.path.join

ignore_dirs=set(['install'])
forbidden_names = set([s for s in ignore_dirs]+['dgbuild'])#forbid dgbuild, to avoid conflict with "dgbuild" python module

import re
_nameval_pattern='^[a-zA-Z][a-zA-Z0-9_]{2,24}$'
_nameval=re.compile(_nameval_pattern).match
_nameval_descr='Valid names follow pattern %s, contain no duplicate underscores and is not one of "%s"'%(_nameval_pattern,
                                                                                                       '", "'.join(forbidden_names))
def pkgname_valid(n):
    return _nameval(n) and not '__' in n and not n in forbidden_names

def parse_depfile(pkgdir):
    filename=os.path.join(pkgdir,conf.package_cfg_file)
    def _err(m=''):
        error.error('Package "%s" has invalid %s file%s'%(os.path.basename(pkgdir),conf.package_cfg_file,m))
    with open(filename,'rt') as fh:
        for iline1,l in enumerate(fh):
            l=l.split('#')[0].strip()
            if l.startswith('package(') and l.endswith(')'):
                l=l[8:-1]
                extdeps=[]
                pkgdeps=[]
                extra_cflags=[]
                extra_ldflags=[]
                extra_incdeps=[]
                current=None
                dyncode=None
                dyncodeoffset=0
                isdynamicpkg=False
                for e in l.split():
                    if e=='USEPKG': current=pkgdeps
                    elif e=='USEEXT': current=extdeps
                    elif e=='EXTRA_COMPILE_FLAGS': current=extra_cflags
                    elif e=='EXTRA_LINK_FLAGS': current=extra_ldflags
                    elif e=='EXTRA_INCDEPS': current=extra_incdeps
                    elif e=='DYNAMICPKG': isdynamicpkg,current=True,None
                    else:
                        if current==None:
                            _err()
                        current += [e]
                if isdynamicpkg:
                    #Consume rest of file, looking for dynamic_builder_begin()/dynamic_builder_end() lines:
                    dyncodedone=False
                    for iline2,l in enumerate(fh):
                        if l.startswith("dynamic_builder_begin()"):
                            if dyncodedone or dyncode is not None:
                                _err(' : unexpected dynamic_builder_begin() statement.')
                            dyncode=''
                            dyncodeoffset=iline1+iline2+2
                            continue
                        if l.startswith("dynamic_builder_end()"):
                            if dyncodedone or dyncode is None:
                                _err(' : unexpected dynamic_builder_end() statement.')
                            dyncodedone=True
                            continue
                        if dyncode is not None and not dyncodedone:
                            dyncode += l
                    if not dyncode:
                        _err(' : Missing code in dynamic_builder_begin()/dynamic_builder_end() block despite being DYNAMICPKG.')
                    if not dyncodedone:
                        _err(' : Missing dynamic_builder_end() statement.')
                    assert isdynamicpkg == bool(dyncode)
                return (set(extdeps),set(pkgdeps),extra_cflags,extra_ldflags,extra_incdeps,dyncode,dyncodeoffset)
        _err(' : missing package() statement.')

from pathlib import Path
def check_case_insensitive_duplication(path_str):
  path = Path(path_str)
  if not path.exists():
    return
  parentContent = [d.lower() for d in os.listdir(path.parent)]
  occurance = parentContent.count(path.name.lower())
  if not occurance == 1:
    raise SystemExit('Directory (and file) names differing only in casing are not allowed, due to being a potential source of error for different file systems. \nProblem occured with '%(path_str))

def check_dir_case_insensitive_duplication(dircontent, path):
  ''' Check if dircontent contains elements differing only in casing'''
  if not len(dircontent) == len({d.lower() for d in dircontent}):
    seen = set()
    for d in [d.lower() for d in dircontent]:
      if d in seen:
        raise SystemExit('Directory (and file) names differing only in casing are not allowed, due to being a potential source of error for different file systems. \nProblem occured with %s in the directory'%(d,path))
      else:
        seen.add(d)
    

def find_pkg_dirs(dirname):
    if os.path.exists(os.path.join(dirname,conf.package_cfg_file)):
        #dir is itself a pkg dir
        return [os.path.realpath(dirname)]
    #dir is not a pkg dir so check sub-dirs:
    pkg_dirs=[]
    dircontent = os.listdir(dirname)
    check_dir_case_insensitive_duplication(dircontent, dirname)
    for item in dircontent:
        if not item or item[0]=='.':
            continue#always ignore hidden dirs
        d=os.path.join(dirname, item)
        if os.path.isdir(d):
            if not item in ignore_dirs:
                pkg_dirs+=find_pkg_dirs(d)
    return pkg_dirs

class Package:
    #Only construct this with the PackageLoader

    #for python3:
    def __lt__(self,o):
        assert self.name != o.name
        return self.name < o.name

    def __init__(self,dirname,basedir,enabled):
        self.dirname = dirname
        self.__haslib=None
        self.reldirname = os.path.relpath(dirname,basedir)
        #self.reldirnamewithparent = os.path.relpath(dirname,basedir+'/..')
        self.name = os.path.basename(dirname)
        assert isinstance(self.name,str)
        assert isinstance(self.dirname,str)
        assert isinstance(self.reldirname,str)
        self.enabled=enabled
        self.is_setup=False
        self._any_parent_changed=False
        self.__deps_names=None
        self.__incs_updated=False
        self.__runnables=set()#to guard against clashes from scripts and app_, etc.

    def register_runnable(self,n):
        if n in self.__runnables:
            error.error('Package "%s" provides multiple sources for the command "%s"'%(self.name,n))
        self.__runnables.add(n)

    def update_incs(self):
        if not self.__incs_updated:
            self.__incs_updated = True
            d=dirs.pkg_dir(self,'libinc')
            import db
            db.db['pkg2inc'][self.name]= langs.headers_in_dir(d) if os.path.isdir(d) else set()

    def setup(self,name2object,extdeps,autodeps,enable_and_recurse_to_deps=False):
        #Ensures cfg file is parsed and direct dep links are established.
        #Recursively setups (and enables) deps if requested.
        if self.is_setup:
            return
        self.is_setup=True
        pcf=join(self.dirname,conf.package_cfg_file)
        ed,pd,cf,lf,extra_incdeps,dynpkgcode,dynoffset=parse_depfile(self.dirname)
        if not self.name in autodeps:
            pd.update(autodeps)
        for e in ed:
            if not e in extdeps:
                error.error('Unknown external dependency "%s" specified in %s'%(e,pcf)+
                          '\nPossible values are: "%s"'%'", "'.join(extdeps))
        self.direct_deps_extnames = ed
        self.direct_deps_pkgnames = pd
        self.direct_clients=set()
        self.extraflags_comp = cf
        self.extraflags_link = lf
        self.extra_include_deps = []
        self.isdynamicpkg = bool(dynpkgcode)
        self.dynamicpkgcode = dynpkgcode
        self.dynamicpkgcode_lineoffset = dynoffset if self.isdynamicpkg else 0
        for eid in extra_incdeps:
            eid=[e.strip() for e in eid.split(':')]
            if len(eid)!=2 or not eid[0] or not eid[1]:
                error.error('Malformed EXTRA_INCDEPS statement in %s'%pcf)
            self.extra_include_deps += [eid]

        #self.enabled_direct_clients = set()#To be set later
        l=[]
        for n in pd:
            o=name2object(n)
            if not o:
                if n in autodeps:
                    error.error('Unknown package "%s" specified as auto dependency'%n)
                else:
                    error.error('Unknown package "%s" specified in %s'%(n,pcf))
            l+=[o]
        self.direct_deps=l
        if enable_and_recurse_to_deps:
            self.enabled=True
            for p in l:
                p.setup(name2object,extdeps,autodeps,True)
        self.__deps=None
        self.__extdeps=None
        self.__deplock=False

    def deps(self):
        #all pkgs we depend on, directly or indirectly.
        if self.__deps!=None:
            return self.__deps
        if self.__deplock:
            if self in self.direct_deps:
                error.error('Package "%s" depends on itself'%self.name)
            else:
                error.error('Detected circular dependency involving package "%s"'%self.name)
        self.__deplock=True
        d=set(self.direct_deps)
        for p in self.direct_deps:
            d.update(p.deps())
        self.__deplock=False
        self.__deps=d
        return d

    def deps_names(self):
        #names all pkgs we depend on, directly or indirectly, including ourselves.
        if self.__deps_names!=None:
            return self.__deps_names
        self.__deps_names = set(p.name for p in self.deps())
        return self.__deps_names

    def extdeps(self):
        #all extdeps we depend on, directly or indirectly.
        if self.__extdeps!=None:
            return self.__extdeps
        self.__extdeps=set(self.direct_deps_extnames)
        for p in self.direct_deps:
            self.__extdeps.update(p.extdeps())
        return self.__extdeps

    def disable(self):
        #Disable direct clients (only if we have actually been setup, otherwise we don't have direct_clients
        self.enabled=False
        _dc = self.direct_clients if hasattr(self,'direct_clients') else []
        for dc in _dc:
            dc.disable()

    def any_parent_changed(self):
        #true if any pkg we depend on indirectly or directly has files_changed=True (set from outside this module)
        return self._any_parent_changed

    def set_parent_changed_recursively(self):
        if self._any_parent_changed:
            return#already done
        self._any_parent_changed=True
        for dc in self.direct_clients:
            dc.set_parent_changed_recursively()

    def has_lib(self):
        if self.__haslib is None:
            self.__haslib = os.path.isdir(dirs.pkg_dir(self,'libsrc'))
        return self.__haslib

    def all_clients(self):
        ic=set()
        ic.update(self.direct_clients)
        for c in self.direct_clients:
            ic.update(c.all_clients())
        return ic

    def info_as_dict(self):
        """Returns info as dictionary, containing not custom classes and suitable for
           pickling. Should be called after setup and after targets have been built, in
            order to guarantee complete info"""
        d = { 'name':self.name,
              'dirname':self.dirname,
              'reldirname':self.reldirname,
              'enabled' : self.enabled }
        if not self.enabled:
            return d
        import db
        d.update({ 'deps_pkgs' : self.deps_names(),
                   'deps_pkgs_direct' : self.direct_deps_pkgnames,
                   'deps_ext' : self.extdeps(),
                   'deps_ext_direct' : self.direct_deps_extnames,
                   'isdynamicpkg' : self.isdynamicpkg,
                   'extraflags_comp' : self.extraflags_comp,
                   'extraflags_link' : self.extraflags_link,
                   'extra_include_deps' : self.extra_include_deps,
                   'has_lib' : self.has_lib(),
                   'clients' : [p.name for p in self.all_clients()],
                   'clients_direct' : [p.name for p in self.direct_clients],
                   'runnables':db.db['pkg2runnables'].get(self.name,set()),
                   'reflogs':db.db['pkg2reflogs'].get(self.name,set()),
        })
        return d

    def dumpinfo(self,autodeps,prefix=''):
        import col
        import env
        all_deps=self.deps()
        width=max(75,len(self.dirname)+30)
        prefix+='==='
        def _format(l):
            if not l:
                return col.darkgrey+'<none>'+col.end
            import formatlist
            return '\n'.join(formatlist.formatlist([(n,col.ok if b else col.bad) for n,b in sorted(l)],width-40,indent_first='',indent_others=prefix+' '*27))
        extdeps_direct = [(e,env.env['extdeps'][e]['present']) for e in self.direct_deps_extnames]
        extdeps_indirect = [(e,env.env['extdeps'][e]['present']) for e in self.extdeps() if not e in self.direct_deps_extnames]
        def nameformat(n):
            return n+'*' if n in autodeps else n
        clients_direct=[(nameformat(p.name),p.enabled) for p in self.direct_clients]
        clients_indirect=[(nameformat(p.name),p.enabled) for p in self.all_clients().difference(self.direct_clients)]
        deps_direct = [(nameformat(p.name),p.enabled) for p in self.direct_deps]
        deps_indirect = [(nameformat(p.name),p.enabled) for p in self.deps() if not p in self.direct_deps]

        s='='*int((width-len('Package information')-2)//2)
        l1='%s Package information %s'%(s,s)
        print(prefix+l1)
        print('%s Package name            : %s%s'%(prefix,_format([(self.name,self.enabled)]),
                                                   (' %s[dynamic package]%s'%(col.lightcyan,col.end) if self.isdynamicpkg else '')))
        print('%s Enabled                 : %s'%(prefix,_format([('Yes' if self.enabled else 'No',self.enabled)])))
        print('%s Relative location       : %s%s%s'%(prefix,col.blue,self.reldirname,col.end))
        print('%s Physical location       : %s%s%s'%(prefix,col.yellow,self.dirname,col.end))
        if self.isdynamicpkg:
            print('%s Dynamic content         : %s%s%s'%(prefix,col.lightcyan,dirs.pkg_dir(self),col.end))
        print('%s Direct relations'%prefix)
        print('%s   Dependencies          : %s'%(prefix,_format(deps_direct)))
        print('%s   Clients               : %s'%(prefix,_format(clients_direct)))
        print('%s   External dependencies : %s'%(prefix,_format(extdeps_direct)))
        print('%s Indirect relations'%prefix)
        print('%s   Dependencies          : %s'%(prefix,_format(deps_indirect)))
        print('%s   Clients               : %s'%(prefix,_format(clients_indirect)))
        print('%s   External dependencies : %s'%(prefix,_format(extdeps_indirect)))
        print(prefix+'='*len(l1))


class PackageLoader:

    def __init__(self,
                 basedirs,
                 extdeps,
                 select_pkg_filter=None,
                 exclude_pkg_filter=None,
                 autodeps=None,
                 load_all=False):
        """
        Locates package directories and inits wrapper Package objects for each.
        It will respect select_pkg_filter or exclude_pkg_filter and attempt to be
        economical about loading the package configuration files. Filters will
        receive package reldirname and basename as input.' #FIXME Milan
        """
        #use load_all to (inefficiently) load all cfg info even when select filter is set (to produce pkg graphs)

        #Note, when select_pkg_filter is set, we do not need to load ALL cfg
        #files, hence the difference in treatment below.
        self.autodeps=set(autodeps if autodeps else [])

        #1) Sanity check arguments:
        #Commented for now, since we want to enable both NOT= and ONLY=
        #variables at the same time. Leaving here in case we encounter
        #unexpected side-effects:
        #if select_pkg_filter and exclude_pkg_filter:
        #    error.error("Can't exclude packages when simultaneously requesting to enable only certain packages")

        #2) Read directory structure in order to find all package directories:
        pkgdirs={}
        for basedir in basedirs:
          check_case_insensitive_duplication(basedir)
          tmp_dirs = find_pkg_dirs(basedir)
          if tmp_dirs:
            pkgdirs.update({d:basedir for d in tmp_dirs})
          elif not basedir==dirs.projdir:
            error.error("No packages found in %s!"%basedir)
          
        #3) Construct Package objects and name->object maps:
        default_enabled = False if select_pkg_filter else True
        n2p={}
        lowercased_pkgs=set()
        pkgs=[]
        for pd,basedir in pkgdirs.items():
            p=Package(pd,basedir,default_enabled)
            pkgs+=[p]
            ln=p.name.lower()
            if p.name in n2p:
                error.error('Duplicate package name: "%s"'%p.name)
            if ln in lowercased_pkgs:
                clashes='" "'.join(p.name for p in pkgs if p.name.lower()==ln)
                error.error('Package names clash when lowercased: "%s" '%clashes)
            lowercased_pkgs.add(ln)
            if not pkgname_valid(p.name):
                error.error('Invalid package name: "%s". %s.'%(p.name,_nameval_descr))
            n2p[p.name]=p
        self.pkgs=pkgs
        self.name2pkg=n2p

        #4) Load contents of all or some of the package config files.
        g=self.name2pkg.get
        def pkg_name2obj(pn):
            return g(pn,None)
        if select_pkg_filter:
            for p in pkgs:
                if select_pkg_filter(p.name,p.dirname):
                    p.setup(pkg_name2obj,extdeps,autodeps,True)
            #always enable the autodeps no matter what:
            if autodeps:
                for p in (pkg_name2obj(ad) for ad in autodeps):
                    p.setup(pkg_name2obj,extdeps,autodeps,True)
        if not select_pkg_filter or load_all:
            for p in pkgs:
                p.setup(pkg_name2obj,extdeps,autodeps)

        #5) setup client links (only to clients which are setup):
        for p in pkgs:
            if p.is_setup:
                for pd in p.direct_deps:
                    pd.direct_clients.add(p)

        #6) Apply exclusion filter:
        if exclude_pkg_filter:
            for p in pkgs:
                if exclude_pkg_filter(p.name,p.dirname):
                    p.disable()

        for p in self.enabled_pkgs_iter():
            p.deps()#must setup deps of all enabled packages, otherwise we can't be sure to detect circular deps

    def enabled_pkgs_iter(self):
        for p in self.pkgs:
            if p.enabled:
                yield p

    def active_deps_extnames(self):
        s=set()
        for e in (p.direct_deps_extnames for p in self.enabled_pkgs_iter()):
            s.update(e)
        return s

    def disable_pkgs_with_extdeps(self,extdeps):
        if not extdeps:
            return#nothing to do
        prev_enabled=set(self.enabled_pkgs_iter())
        for p in self.pkgs:
            if p.enabled and not extdeps.isdisjoint(p.direct_deps_extnames):
                p.disable()
        return prev_enabled.difference(set(self.enabled_pkgs_iter()))
