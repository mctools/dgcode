import os,re
import conf, target_base, env, dirs, langs, utils, includes, col, db, error
join=os.path.join

has_realpath = os.path.exists('/usr/bin/realpath')

class TargetBinaryObject(target_base.Target):
    def __init__(self,pkg,subdir,bn,e,lang,shlib,cflags,possible_privincs):
        langinfo=env.env['system']['langs'][lang]
        self.pkglevel=False#needed by the corresponding TargetBinary object
        self.pkgname=pkg.name
        self._fn=bn+e
        self._lang=lang
        self._shlib=shlib
        d=dirs.makefile_pkg_cache_dir(pkg,'objs',subdir)
        self.name=join(d,langinfo['name_obj']%bn)
        fn=bn+e
        sf=dirs.makefile_pkg_dir(pkg,subdir,fn)
        self.__pklfile=dirs.pkg_cache_dir(pkg,'objs',subdir,fn+'_deps.pkl')
        self.deps=[dirs.makefile_blddir(self.__pklfile)]
        self.deps+=['${LANG}/%s'%lang,'%s_prepinc'%pkg.name,sf]#FIXME perhaps on /cxxpch in case of cxx? (if has pch reqs)
        extra_flags=['-I%s'%dirs.makefile_pkg_cache_dir(pkg,'inc'),'-DPACKAGE_NAME=%s'%pkg.name]

        if cflags:
            extra_flags+=cflags
        if shlib:
            extra_flags+=['-fPIC']#Could we get this from cmake already??
        for extdep in pkg.extdeps():
            extra_flags+=['${CFLAGS_%s_%s}'%(('CXX' if lang=='cxx' else 'C'),extdep)]
            self.deps+=['${EXT}/%s'%extdep]


        if pkg.extraflags_comp:
            extra_flags+=pkg.extraflags_comp

        contains_message=True
        self.code=['@if [ ${VERBOSE} -ge 0 ]; then echo "  %sBuilding %s/%s/%s%s"; fi'%(col.bldcol('objectfile'),
                                                                                        pkg.name,
                                                                                        subdir,
                                                                                        os.path.basename(self.name),
                                                                                        col.bldend),
                   'mkdir -p %s'%d,
                   langinfo['create_obj']%(' '.join(extra_flags),'`realpath %s`'%sf if has_realpath else sf,self.name)]

        priv,pub = includes.find_includes(dirs.pkg_dir(pkg,subdir,fn),pkg)

        # NOTE THAT FOLLOWING LINES OF CODE ARE SIMILAR TO ONES IN tfact_headerdeps.py !!!

        #deal with the private dependencies as well as public deps to same pkg
        #here, postpone the other public ones until setup() when other packages are available.
        self._pkloffset=len(self.deps)
        if priv and possible_privincs:
            for p in priv:
                if p in possible_privincs:
                    self.deps+=['${TRG}/%s__%s__%s'%(pkg.name,subdir,p)]
        if pub:
            pkg.update_incs()
            incs=db.db['pkg2inc'][pkg.name]
            self.deps += ['${TRG}/%s__libinc__%s'%(p,f) for (p,f) in pub if (p==self.pkgname and f in incs)]
            self._pub = [(p,f) for (p,f) in pub if p!=self.pkgname]
        else:
            self._pub=None

    def setup(self,pkg):
        if self._pub:
            # NOTE THAT FOLLOWING LINES OF CODE ARE SIMILAR TO ONES IN tfact_headerdeps.py !!!
            p2i=db.db['pkg2inc']
            d=pkg.deps_names()
            self.deps+=['${TRG}/%s__libinc__%s'%(p,f) for (p,f) in self._pub if (p in d and f in p2i.get(p,[]))]
        pklcont=set(self.deps[self._pkloffset:])
        utils.mkdir_p(os.path.dirname(self.__pklfile))
        utils.update_pkl_if_changed(pklcont,self.__pklfile)

class TargetBinary(target_base.Target):

    def __init__(self,pkg,subdir,lang,shlib,object_targets,instsubdir,namefct,descrfct=None,checkfct=None):
        db.db['pkg2parts'][pkg.name].add(subdir)
        langinfo=env.env['system']['langs'][lang]
        self.pkgname=pkg.name
        self._subdir=subdir

        pattern='name_lib' if shlib else 'name_exe'
        filename=namefct(pkg,subdir,langinfo[pattern])
        if checkfct:
            checkerrmsg = checkfct(pkg,subdir,filename)
            if checkerrmsg:
                error.error(checkerrmsg)

        if subdir.startswith('app_'):
            pkg.register_runnable(filename)
            db.db['pkg2runnables'].setdefault(pkg.name,set()).add(filename)

        dcol,descr = descrfct(pkg,subdir,filename) if descrfct else ('','%s/%s'%(pkg.name,filename))
        dcolend = col.bldend if dcol else ''
        self.name='%s_%s'%(pkg.name,subdir)

        self.deps=[]
        for ot in object_targets:
            self.deps.append(ot.name)
        self.deps.sort()
        sobjs=' '.join(self.deps)
        cachedir=dirs.pkg_cache_dir(pkg,'objs')
        utils.mkdir_p(cachedir)
        objlistfile=join(cachedir,'objs_%s.txt'%subdir)#to make sure we rebuild when removing a src file
        self.deps+=[dirs.makefile_blddir(objlistfile)]#'%s_prepinc'%pkg.name]
        extra_flags=[]
        extra_flags+=['%s']#This is used in self.setup() to add in library deps.
        for extdep in pkg.extdeps():
            extra_flags+=['${LDFLAGS_%s} ${LDFLAGS_%s_%s_%s}'%(extdep,extdep,('LIB' if shlib else 'EXE'),lang)]
        if pkg.extraflags_link:
            extra_flags+=pkg.extraflags_link
        pattern = 'create_lib' if shlib else 'create_exe'
        rpath_pattern = 'rpath_flag_lib' if shlib else 'rpath_flag_exe'
        extra_flags+=[ langinfo[rpath_pattern]%join('${INST}','lib') ]
        extra_flags+=[ langinfo[rpath_pattern]%join('${INST}','lib','links') ]
        if not os.path.exists(objlistfile) or open(objlistfile).read()!=sobjs:
            open(objlistfile,'w').write(sobjs)
        d=join('${INST}',instsubdir)
        contains_message=True
        self.code=['@if [ ${VERBOSE} -ge 0 ]; then echo "  %sCreating %s%s"; fi'%(dcol,descr,dcolend),
                   'mkdir -p %s'%d,
                   langinfo[pattern]%(' '.join(extra_flags),
                                      sobjs,
                                      join(d,filename))]

    def setup(self,pkg):
        l=[]
        if self._subdir!='libsrc':
            if pkg.has_lib():
                l+=[conf.libldflag(pkg)]
            self.deps+=['%s_libavail'%self.pkgname]
        for p in pkg.deps():
            if p.has_lib():
                l+=[conf.libldflag(p)]
            self.deps+=['%s_libavail'%p.name]
        if l:
            l.insert(0, '-L${INST}/lib')
        self.code[-1]=self.code[-1]%(' '.join(l))

import tfact_headerdeps


def create_tfactory_binary(instsubdir=None,pkglib=False,shlib=False,allowed_langs=None,namefct=None,flagfct=None,descrfct=None,checkfct=None):
    if not allowed_langs:
        allowed_langs = langs.langs
    if pkglib:
        shlib=True
        instsubdir='lib'
    blainstsubdir=instsubdir#weird that we need this to propagate the variables into the next function
    def tfact_bin(pkg,subdir):
        instsubdir=blainstsubdir
        if '%s' in instsubdir:
            instsubdir = instsubdir%pkg.name
        srcs=[]
        hdrs=[]
        langspresent=set()
        for f in utils.listfiles(dirs.pkg_dir(pkg,subdir),ignore_logs=subdir.startswith('app_')):
            n,e=os.path.splitext(f)
            assert n
            langsrc = langs.srcext2lang.get(e,None)
            if langsrc:
                langspresent.add(langsrc)
                srcs+=[(n,e,langsrc)]
                continue
            langhdr = None if langsrc else langs.hdrext2lang.get(e,None)
            if langhdr:
                langspresent.add(langhdr)
                hdrs+=[(n,e,langhdr)]
                continue
            error.error("File is neither src or header file of allowed language: %s"%f)

        if not srcs:
            error.error("No source files found in %s/%s (remove this directory if not needed)"%(pkg.name,subdir))
        if len(langspresent)!=1:
            assert len(langspresent)>1
            error.error("Files for multiple languages found in %s/%s"%(pkg.name,subdir))
        langspresent=langspresent.pop()
        if not langspresent in allowed_langs:
            error.error("Files of language '%s' found in %s/%s where only allowed languages are '%s'"%(langspresent,
                                                                                                       pkg.name,subdir,
                                                                                                       ';'.join(allowed_langs)))
        tgts=[]
        extra_cflags=flagfct(pkg,subdir) if flagfct else []

        possible_privincs = langs.headers_in_dir(dirs.pkg_dir(pkg,subdir))
        for bn,e,lang in srcs:
            tgts+=[TargetBinaryObject(pkg,subdir,bn,e,lang,
                                      shlib=shlib,cflags=extra_cflags,
                                      possible_privincs=possible_privincs)]
        tb=TargetBinary(pkg,subdir,langspresent,
                        shlib=shlib,object_targets=tgts,
                        instsubdir=instsubdir,namefct=namefct,descrfct=descrfct,
                        checkfct=checkfct)
        if pkglib:
            pkg.libtarget=tb
        tgts+=[tb]

        tgts += tfact_headerdeps.tfactory_headerdeps(pkg,subdir)

        return tgts
    return tfact_bin


