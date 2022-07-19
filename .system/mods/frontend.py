#Build script which is intended to make the following steps smooth, painless and efficient for users:
#
#  1) Detect environment with cmake
#  2) Configuration step
#  3) Build step
#  4) Install step
#  5) Test launch.
#
# Most importantly, it is intended that minimal cpu time is used to needless redo work in steps 1-4
# above when user changes code or configuration.

import sys,os,glob,pipes

progname=os.path.basename(sys.argv[0])
prefix=progname+': '
v=sys.version_info[0:2]
if v<(3,6) or v>=(4,0):
    print (prefix+"Error: Python not found in required version. The python version must be at least 3.6 but less than 4.0 (check with python -V).")
    sys.exit(1)

from optparse import OptionParser,OptionGroup#FIXME: deprecated, use argparse instead!

isdir=os.path.isdir
exists=os.path.exists
def isfile(s): return os.path.exists(s) and not os.path.isdir(s)
osx=not isfile('/proc/cpuinfo')

#########################
### Parse arguments  ####
#########################

import dirs,utils,error
rel_blddir=os.path.relpath(dirs.blddir)
rel_instdir=os.path.relpath(dirs.installdir)
rel_testdir=os.path.relpath(dirs.testdir)

if not any([os.path.realpath(os.getcwd()).startswith(str(d)) for d in [dirs.fmwkdir.parent.parent, *dirs.pkgsearchpath]]):
    utils.err(['This instance of %s is associated with'%progname,'',
               '      dgcode dir:  %s'%dirs.fmwkdir.parent.parent,
               '      package search path: %s'%str(dirs.pkgsearchpath[0]),
               *['                         : %s'%str(d) for d in dirs.pkgsearchpath[1:]], '',
               'and must only be invoked in those directories or their subdirs.'])

proj_pkg_selection_enabled = os.environ.get('DGCODE_ENABLE_PROJECTS_PKG_SELECTION_FLAG','')

def parse_args():

    #Prepare parser:
    parser = OptionParser(usage='%prog [options]')

    group_build = OptionGroup(parser, "Controlling the build","The build will be carried out"
                              " in the %s/ directory and after a successful build it"
                              " will install the results in %s/. In addition to"
                              " configuration variables, the follow options can be used to fine-tune"
                              " the build process."%(rel_blddir,rel_instdir))
    group_build.add_option("-j", "--jobs",
                           type="int", dest="njobs", default=0,
                           help="Use up to N parallel processes",metavar="N")
    group_build.add_option("-v", "--verbose",
                           action='store_true', dest="verbose", default=False,
                           help="Enable more verbosity")
    group_build.add_option("-q", "--quiet",
                           action='store_true', dest="quiet", default=False,
                           help="Less verbose. Silence even some warnings")
    group_build.add_option("-e", "--examine",
                           action='store_true', dest="examine", default=False,
                           help="Force (re)examination of environment")
    group_build.add_option("-i", "--insist",
                           action="store_true", dest="insist", default=False,
                           help="Insist on reconf/rebuild/reinstall from scratch")#nb: does not forget CMake args!
    parser.add_option_group(group_build)

    group_cfgvars = OptionGroup(parser, "Setting configuration variables",
                                "You can set variables to be used during the configuration stage. Once"
                                " set, the variables will be remembered and do not need to be specified"
                                " upon next invocation of %s. Use the -s switch for more info."%progname)
    group_cfgvars.add_option("-s", "--show",
                             action='store_true', dest="show", default=False,
                             help="Show stored configuration variables and exit")
    group_cfgvars.add_option("-f", "--forget",
                             action='store_true', dest="forget", default=False,
                             help="Forget stored configuration variables and exit")
    group_cfgvars.add_option("-r", "--release",
                             action='store_true', dest="release", default=False,
                             help="Shortcut for CMAKE_BUILD_TYPE=RELEASE")
    group_cfgvars.add_option("-d", "--debug",
                             action='store_true', dest="debug", default=False,
                             help="Shortcut for CMAKE_BUILD_TYPE=DEBUG")
    parser.add_option_group(group_cfgvars)

    group_pkgselect = OptionGroup(parser, "Selecting what packages to enable",
                                  "The flags below provide a convenient alternative to"
                                  " direct modification of the configuration variable named \"ONLY\". Default is to enable all packages.")
    group_pkgselect.add_option("-a","--all",action='store_true', default=False,dest='enableall',
                             help="Enable *all* packages.")

    if proj_pkg_selection_enabled:
      group_pkgselect.add_option("-p","--project",
                             action='store', dest="project", default='',metavar='PROJ',
                             help='Enable packages in selected projects (under packages/ '
                             ' or folders defined by the DGCODE_PKG_PATH environment variable).'
                             ' The selection can use wildcards and comma separation.')
#    group_pkgselect.add_option("-k","--pkg",
#                             action='store', dest="pkgs", default='',metavar='PKG',
#                             help='Enable these packages in addition to those selected by'
#                             ' --project (can use wildcards and comma separation).')
#    #TODO: Make *exclusion* easy as well (Need both NOT and ONLY vars to work at the same time).

    parser.add_option_group(group_pkgselect)

    group_query = OptionGroup(parser, "Query options")

    group_query.add_option("--pkginfo",
                           action="store", dest="pkginfo", default='',metavar='PKG',
                           help="Print information about package PKG")
    group_query.add_option("--incinfo",
                           action="store", dest="incinfo", default='',metavar='CFILE',
                           help="Show inclusion relationships for the chosen CFILE. CFILE must be a C++ or C file in the framework. Optionally multiple files can be specified using comma-separation and wildcards (\"*\').")
    group_query.add_option("--pkggraph",
                           action="store_true", dest="pkggraph", default=False,
                           help="Create graph of package dependencies")
    group_query.add_option("--activegraph",
                           action="store_true", dest="pkggraph_activeonly", default=False,
                           help="Create graph for enabled packages only")
    group_query.add_option("--grep",
                           action="store", dest="grep", default='',metavar='PATTERN',
                           help="Grep files in packages for PATTERN")
    group_query.add_option("--grepc",
                           action="store", dest="grepc", default='',metavar='PATTERN',
                           help="Like --grep but show only count per package")
    group_query.add_option("--find",
                           action="store", dest="find", default=None, metavar='PATTERN',
                           help="Search for file and path names matching PATTERN")
    parser.add_option_group(group_query)

    group_other = OptionGroup(parser, "Other options")
    group_other.add_option("-t", "--tests",
                           action="store_true", dest="runtests", default=False,
                           help="Run tests after the build")
    group_other.add_option("--testexcerpts",
                           type="int", dest="nexcerpts", default=0,
                           help="Show N first and last lines of each log file in failed tests",metavar="N")
    group_other.add_option("-c", "--clean",
                           action='store_true', dest="clean", default=False,
                           help="Remove %s and %s directories and exit"%(rel_blddir,rel_instdir))
    group_other.add_option("--replace",
                           action="store", dest="replace", default=None, metavar='PATTERN',
                           help="Global search and replace in packages via pattern like '/OLDCONT/NEWCONT/' (use with care!)")
    group_other.add_option("--install",metavar="DIR",
                           action="store", type="string", dest="install",default='',
                           help="Install to DIR after the build")
    group_other.add_option("--removelock",
                           action='store_true', dest="removelock", default=False,
                           help="Force removal of lockfile")

    parser.add_option_group(group_other)

    #Actually parse arguments, taking care that the ones of the form VAR=value
    #must be interpreted as user configuration choices, most passed to cmake.
    new_cfgvars={}
    args_unused=[]
    (opt, args) = parser.parse_args(sys.argv[1:])
    for a in args:
        t=a.split('=',1)
        if len(t)==2 and t[0] and not ' ' in t[0]:
            if t[0] in new_cfgvars:
                parser.error('Configuration variable %s supplied multiple times'%t[0])
            new_cfgvars[t[0]]=t[1]
        else:
            args_unused+=[a]
    del args

    #Remove arguments (to make sure nothing in backend code depends on them)
    del sys.argv[1:]

    opt._querypaths=[]
    if opt.grepc:
        if opt.grep:
            parser.error("Don't supply both --grep and --grepc")
        opt.grep=opt.grepc
        opt.grepc=bool(opt.grepc)

    if new_cfgvars and (opt.forget or opt.clean or opt.pkginfo or opt.grep or opt.incinfo):
        parser.error("Don't supply <var>=<val> arguments together with --clean, --forget, --grep, --incinfo or --pkginfo flags")

    if opt.pkggraph_activeonly:
        opt.pkggraph=True

    #rel/debug flags:
    if opt.release and opt.debug:
        parser.error('Do not supply both --debug and --release flags')
    if (opt.release or opt.debug) and 'CMAKE_BUILD_TYPE' in new_cfgvars:
        parser.error('Do not supply --debug or --release flags while setting CMAKE_BUILD_TYPE directly')
    if opt.release: new_cfgvars['CMAKE_BUILD_TYPE']='RELEASE'
    if opt.debug: new_cfgvars['CMAKE_BUILD_TYPE']='DEBUG'

    if proj_pkg_selection_enabled:
      if opt.project and opt.enableall:
        parser.error('Do not specify both --all and --project')

      if opt.project:
          #TODO: This is rather specific to our way of structuring directories...
          if 'ONLY' in new_cfgvars:
              parser.error('Do not set ONLY=... variable when supplying --project flag')
          #if 'NOT' in new_cfgvars:
          #    parser.error('Do not set NOT=... variable when supplying --project flag')
          projs = set(e.lower() for e in opt.project.split(','))
          extra='Framework::*,'
          if 'val' in projs:
              projs=[a for a in projs if a.lower() != 'val']
              extra+='Validation::*,'
          if not projs:
              extra=extra[:-1]#remove comma
          new_cfgvars['ONLY']='%s%s'%(extra,','.join('Projects::%s*'%p for p in projs))
          if not 'NOT' in new_cfgvars:
              new_cfgvars['NOT']=''

    #if opt.pkgs and opt.enableall:
        #parser.error('Do not specify both --all and --project')

#    if opt.pkgs:
#        if 'ONLY' in new_cfgvars:
#            parser.error('Do not set ONLY=... variable when supplying --pkg flag')
#        if 'NOT' in new_cfgvars:
#            parser.error('Do not set NOT=... variable when supplying --pkg flag')
##    if opt.pkgs:
##        pkgs = set(e.lower() for e in opt.pkg.split(','))
##        _only=new_cfgvars.get('ONLY','')
##        new_cfgvars['ONLY']='%s%s'%(extra,','.join('Projects/%s*'%p for p in projs))
##        new_cfgvars['NOT']=''
##     .... todo...

    if opt.enableall:
        if 'ONLY' in new_cfgvars:
            parser.error('Do not set ONLY=... variable when supplying --all flag')
        #if 'NOT' in new_cfgvars:
        #    parser.error('Do not set NOT=... variable when supplying --all flag')
        if not 'NOT' in new_cfgvars:
            new_cfgvars['NOT']=''
        new_cfgvars['ONLY']='*'#this is how we make sure --all is remembered

    query_mode_withpathzoom_n = sum(int(bool(a)) for a in [opt.grep,opt.replace,opt.find])
    query_mode_n = query_mode_withpathzoom_n + sum(int(bool(a)) for a in [opt.pkggraph,opt.pkginfo,opt.incinfo])
    if int(opt.forget)+int(opt.show)+int(opt.clean)+ query_mode_n > 1:
        parser.error("More than one of --clean, --forget, --show, --pkggraph, --pkginfo, --grep, --grepc, --replace, --find, --incinfo specified at the same time")
    opt.query_mode = query_mode_n > 0
    if query_mode_withpathzoom_n > 0:
        for a in args_unused:
            qp=os.path.abspath(os.path.realpath(a))
            if not any([qp.startswith(str(d)) for d in dirs.pkgsearchpath]):
                parser.error("grep/find/replace/... can only work on directories below %s"%dirs.pkgsearchpath) #TODO ' '.join(pkgsearchpath) might look nicer
            gps=[d for d in glob.glob(qp) if os.path.isdir(d)]
            if not gps:
                parser.error("no directory matches for '%s'"%a)
            for d in sorted(gps):
              opt._querypaths+=['%s/'%os.path.relpath(d,codedir) for codedir in dirs.pkgsearchpath if d.startswith(codedir)]
        args_unused=[]

    if args_unused:
        parser.error("Unrecognised arguments: %s"%' '.join(args_unused))

    if opt.verbose and opt.quiet:
        parser.error("Do not supply both --quiet and --verbose flags")

    if opt.install:
        opt.install=os.path.realpath(os.path.expanduser(opt.install))
        i=opt.install
        if (isdir(i) and not utils.isemptydir(i)) or isfile(i) or (not exists(i) and not exists(os.path.dirname(i))):
            parser.error('Please supply non-existing or empty directory to --install')

    return parser,opt,new_cfgvars

parser,opt,new_cfgvars=parse_args()

#setup lockfile:
if opt.removelock and dirs.lockfile.exists():
    os.remove(dirs.lockfile)
if not hasattr(utils,'mkdir_p'):
    print ("WARNING: utils does not have mkdir_p attribute. Utils module is in file %s and syspath is %s"%(utils.__file__,sys.path))
##utils.mkdir_p(os.path.dirname(dirs.lockfile))
if dirs.lockfile.exists():
    utils.err('Presence of lock file indicates competing invocation of %s. Force removal with %s --removelock if you are sure this is incorrect.'%(progname,progname))
dirs.create_bld_dir()
utils.touch(dirs.lockfile)

assert dirs.lockfile.exists()
def unlock():
    from os import remove
    from dirs import lockfile
    if lockfile.exists():
        remove(lockfile)
import atexit
atexit.register(unlock)

from conf import check_install_dir_indicator, check_build_dir_indicator

if opt.clean:
    if dirs.blddir.is_dir() or dirs.installdir.is_dir() or dirs.testdir.is_dir():
        if not opt.quiet:
            print (prefix+"Removing temporary build, install and test directories and forgetting stored CMake args. Exiting.")
       
        if check_install_dir_indicator(dirs.installdir):
          utils.rm_rf(dirs.installdir)
        if check_build_dir_indicator(dirs.blddir):
          utils.rm_rf(dirs.blddir)
        utils.rm_rf(dirs.testdir)
    else:
        if not opt.quiet:
            print (prefix+"Nothing to clean. Exiting.")
    sys.exit(0)
try:
    old_cfgvars = utils.pkl_load(dirs.varcache)
except IOError:
    old_cfgvars = {}

if opt.forget:
    if old_cfgvars:
        os.remove(dirs.varcache)
        if not opt.quiet:
            print ("%sCleared %i configuration variable%s."%(prefix,len(old_cfgvars),'' if len(old_cfgvars)==1 else 's'))
    else:
        if not opt.quiet:
            print (prefix+"No configuration variables set, nothing to clear.")
    sys.exit(0)

#combine old and new config vars:
cfgvars=dict((k,v) for k,v in new_cfgvars.items() if v)
for k,v in old_cfgvars.items():
    if not k in new_cfgvars:
        cfgvars[k]=v

#Make sure that if nothing is specified, we compile ALL packages, 
# or just Framework packages if project package selection is enabled:
if not 'NOT' in cfgvars and not 'ONLY' in cfgvars: #and not opt.pkgs
  if not proj_pkg_selection_enabled:
    cfgvars['ONLY'] = '*'
  elif not opt.project and not opt.enableall:
    cfgvars['NOT'] = 'Projects::*'


#Old check, we try to allow both variables now:
#if 'ONLY' in cfgvars and 'NOT' in cfgvars:
#    parser.error('Can not set both ONLY and NOT variables simultaneously. Unset at least one by ONLY="" or NOT=""')

#Detect changes to system cmake or python files and set opt.examine or opt.insist as appropriate.
import mtime
systs = (mtime.mtime_cmake(),mtime.mtime_pymods())
try:
    oldsysts = utils.pkl_load(dirs.systimestamp_cache)
except IOError:
    opt.insist=True
    oldsysts = (None,None)

if os.environ.get('DGCODE_ALLOWSYSDEV',''):
    #prevent changes in system from triggering rebuilds (for careful system
    #development use):
    systs = oldsysts

if not opt.insist and oldsysts!=systs:
    if oldsysts[0]!=systs[0]: opt.examine = True
    if oldsysts[1]!=systs[1]: opt.insist = True

if dirs.envcache.exists():
  envdict=utils.pkl_load(dirs.envcache)
  #insist rebuilding from scratch if install dir was changed since the build dir was last used
  if not envdict['system']['volatile']['misc']['DGCODE_INSTALL_DIR_RESOLVED'] == str(dirs.installdir):
    opt.insist = True

if opt.insist:
  if check_install_dir_indicator(dirs.installdir):
    utils.rm_rf(dirs.installdir)
  if check_build_dir_indicator(dirs.blddir):
    utils.rm_rf(dirs.blddir)
  dirs.create_bld_dir()

utils.pkl_dump(cfgvars,dirs.varcache)
utils.pkl_dump(systs,dirs.systimestamp_cache)

if opt.show:
    if cfgvars:
        if not opt.quiet:
            print ('%sCurrently the following configuration variables are set:'%prefix)
        if not opt.quiet:
            print (prefix)
        #print the values even when quiet:
        for k,v in sorted(cfgvars.items()):
            print ('%s    %s=%s'%(prefix,k,pipes.quote(v)))
    else:
        if not opt.quiet:
            print ('%sCurrently no configuration variables are set.'%prefix)
    if not opt.quiet:
        print (prefix)
        print ('%sTo modify this you can (note this is mostly for expert users!):'%prefix)
        print (prefix)
        print ('%s  - set variable by supplying VAR=VAL arguments to %s'%(prefix,progname))
        print ('%s  - unset a variable by setting it to nothing (VAR=)'%(prefix))
        print ('%s  - unset all variables by running %s with --forget (but note that'%(prefix,progname))
        print ('%s    the ONLY variable is special and defaults to "Framework::*")'%prefix)
        print (prefix)
        print ('%sThese are the variables with special support in %s'%(prefix,progname))
        print (prefix)
        print ('%s  - DG_EXTRA_CFLAGS  : Extra compilation flags to be passed to the compiler'%prefix)
        print ('%s                       Example: DG_EXTRA_CFLAGS=-Wshadow'%prefix)
        print ('%s  - DG_EXTRA_LDFLAGS : Extra link flags to be passed to the linker'%prefix)
        print ('%s  - DG_RELAX         : Set to 1 to temporarily ignore compiler warnings'%prefix)
        print ('%s  - ONLY             : Enable only certain packages.'%prefix)
        print ('%s                       Example: ONLY="Framework::*,BasicExamples"'%prefix)
        print ('%s  - NOT              : Disable certain packages.'%prefix)
        print ('%s  - Geant4=0, ROOT=0, etc.. : Force a given external dependency to be'%prefix)
        print ('%s                              considered as absent, even if present.'%prefix)
    sys.exit(0)

def create_filter(pattern):
    #Patterns separated with ; or ,.
    #
    #A '/' char indicates a pattern to be tested versus the path to the package,
    #otherwise it will be a test just against the package name.
    #
    #matching is done case-insensitively via the fnmatch module

    aliases = dirs.pkgdir_aliases
    def expand_alias(part):
      if not '::' in part:
        return part
      if part.split('::')[0].lower() in aliases:
        return os.path.join(aliases[part.split('::')[0].lower()], part.split('::')[1])
      else:
        print(prefix+"Error: Can't find directory alias for %s in %s"%(part.split('::')[0],part))
        sys.exit(1)

    # Separate patterns, and expand directory aliases
    pattern_parts = [expand_alias(p) for p in pattern.replace(';',',').split(',') if p]

    import fnmatch,re
    namepatterns = []
    dirpatterns = []
    for p in pattern_parts:
      if not '/' in p:
        namepatterns.append(re.compile(fnmatch.translate(p.lower())).match)
      elif p.startswith('/'):
        dirpatterns.append(re.compile(fnmatch.translate(p.lower())).match)
      else:
        # create pattern for ALL paths in the pkg search path
        dirpatterns.extend([re.compile(fnmatch.translate( ('%s/*/%s'%(d,p)).lower() )).match for d in dirs.pkgsearchpath])

    def the_filter(pkgname,absdir):
        for p in namepatterns:
            if p(pkgname.lower()): return True
        for d in dirpatterns:
            if d(absdir.lower()): return True
        return False

    return the_filter

select_filter=create_filter(cfgvars['ONLY']) if 'ONLY' in cfgvars else None
exclude_filter=create_filter(cfgvars['NOT']) if 'NOT' in cfgvars else None

cmakeargs=[pipes.quote('%s=%s'%(k,v)) for k,v in cfgvars.items() if not k in set(['ONLY','NOT'])]
cmakeargs.sort()
autodisable = True

import backend

err_txt,unclean_exception = None,None
try:
    pkgloader = backend.perform_configuration(cmakeargs=cmakeargs,
                                              select_filter=select_filter,
                                              exclude_filter=exclude_filter,
                                              autodisable = autodisable,
                                              force_reconf=opt.examine,
                                              load_all_pkgs = opt.query_mode,
                                              prefix=prefix,
                                              quiet=opt.quiet,
                                              verbose=opt.verbose)
except KeyboardInterrupt:
    err_txt = "Halted by user interrupt (CTRL-C)"
except error.CleanExit as ce:
    #errors already handled, exit directly:
    sys.exit(ce._the_ec)
except Exception as e:
    err_txt=str(e)
    if not err_txt: err_txt='<unknown error>'
    if not isinstance(e,error.Error): unclean_exception = e
except SystemExit as e:
    if str(e)!="knownhandledexception":
        err_txt = "Halted by unexpected call to system exit!"
        unclean_exception = e

if err_txt:
    pr="\n\nERROR during configuration:\n\n  %s\n\nAborting."%(err_txt.replace('\n','\n  '))
    print (pr.replace('\n','\n%s'%prefix))
    #make all packages need reconfig upon next run:
    import db
    db.db['pkg2timestamp']={}
    db.save_to_file()
    #exit (with ugly traceback if unknown type of exception):
    if unclean_exception:
        error.print_traceback(unclean_exception,prefix)
    sys.exit(1)

def query_pkgs():
    l=[]
    for p in sorted(pkgloader.pkgs):
        if not opt._querypaths:
            l+=[p]
        else:
            for qp in opt._querypaths:
                if (p.reldirname+'/').startswith(qp):
                    l+=[p]
                    break
    return l

if opt.grep:
    qp=query_pkgs()
    print ("Grepping %i packages for pattern %s\n"%(len(qp),opt.grep))
    n=0
    import grep
    for pkg in qp:
        n+=grep.grep(pkg,opt.grep,countonly=opt.grepc)
    print ("\nFound %i matches"%n)
    sys.exit(0)

if opt.replace:
    qp=query_pkgs()
    p=opt.replace
    import replace
    search_pat,replace_pat = replace.decode_pattern(opt.replace)
    if not search_pat:
        parser.error("Bad syntax in replacement pattern: %s"%p)
    print ("\nReplacing all \"%s\" with \"%s\""%(search_pat,replace_pat))
    n = 0
    for pkg in qp:
        if not pkg.isdynamicpkg:#TODO: Can we somehow support dynamic packages?
            n+=replace.replace(pkg,search_pat,replace_pat)
    print ("\nPerformed %i replacements"%n)
    sys.exit(0)

if opt.find:
    qp=query_pkgs()
    p=opt.find
    import find#fnmatch!!
    print ("\nFinding all files and paths matching \"%s\"\n"%(p))
    n = 0
    for pkg in qp:
        n+=find.find(pkg,p)
    print ("\nFound %i matches"%n)
    sys.exit(0)

if opt.incinfo:
    def _val_incinfofn(fn):
        if '*' in fn:
            #try to expand wildcards:
            fff = sorted(glob.glob(fn))
            if fff:
                return sum((_val_incinfofn(ff) for ff in fff),[])
        if not exists(fn):
            #Could be of form "pkgname/subdir/file. If so, expand pkgname part
            #to full path to package:
            i=fn.split(os.path.sep)
            pkg=pkgloader.name2pkg.get(i[0],None) if len(i)==3 else None
            if pkg:
                return _val_incinfofn(dirs.pkg_dir(pkg,i[1],i[2]))
            else:
                parser.error("File not found: %s"%fn)
        if isdir(fn):
            parser.error("Not a file: %s"%fn)
        fn=os.path.abspath(os.path.realpath(fn))
        if not any([fn.startswith(d) for d in dirs.pkgsearchpath]):
        #if not fn.startswith(os.path.abspath(dirs.codedir)):#TODO: This currently fails for dynamic packages!
            parser.error("File must be located under %s"%dirs.pkgsearchpath)
        return [fn]#expands to a single file
    import incinfo
    fnsraw = opt.incinfo.split(',') if ',' in opt.incinfo else [opt.incinfo]
    fns = sum((_val_incinfofn(fnraw) for fnraw in fnsraw),[])
    #remove duplicates (relies on seen.add returning None)
    seen=set()
    fns = [fn for fn in fns if not (fn in seen or seen.add(fn))]
    #Dispatch to backend:
    if len(fns)==1:
        incinfo.provide_info(pkgloader,fns[0])
    else:
        incinfo.provide_info_multifiles(pkgloader,fns)
    sys.exit(0)

if opt.pkginfo:
    pkg=pkgloader.name2pkg.get(opt.pkginfo,None)
    if not pkg:
        utils.err('Unknown package "%s"'%opt.pkginfo)
    else:
        pkg.dumpinfo(pkgloader.autodeps,prefix)
        sys.exit(0)

if opt.pkggraph:
    dotfile=dirs.blddir / 'pkggraph.dot'
    import dotgen
    dotgen.dotgen(pkgloader,dotfile,enabled_only=opt.pkggraph_activeonly)
    if not opt.quiet:
        print ('%sPackage dependencies in graphviz DOT format has been generated in %s'%(prefix,dotfile))
    ec=utils.system('dot -V > /dev/null 2>&1')
    if ec:
        if not opt.quiet:
            print (prefix+'Warning: command "dot" not found or ran into problems.')
            print (prefix+'Please install graphviz to enable graphical dependency displays')
        sys.exit(1)
    ec=utils.system('unflatten -l3 -c7 %s|dot -Tpng -o%s/pkggraph.png'%(dotfile,dirs.blddir))
    if ec or not isfile('%s/pkggraph.png'%dirs.blddir):
        if not opt.quiet:
            print (prefix+'Error: Problems with dot command while transforming pkggraph.dot to pkggraph.png')
        sys.exit(1)
    else:
        if not opt.quiet:
            print (prefix+'Package dependencies in PNG format has been generated in %s/pkggraph.png'%dirs.blddir)
    sys.exit(0)


if not opt.njobs:
    import cpudetect
    opt.njobs=cpudetect.auto_njobs(prefix)

#VERBOSE:
# -1: always quiet
#  0: only warnings
#  1: extra verbose printouts
if opt.verbose: extramakeopts=' VERBOSE=1'
elif opt.quiet: extramakeopts=' VERBOSE=-1'
else: extramakeopts=''
if not opt.quiet:
    print (prefix+"Configuration completed => Launching build with %i parallel processes"%opt.njobs)
ec=utils.system("cd %s && make --warn-undefined-variables -f Makefile -j%i%s"%(dirs.makefiledir,opt.njobs,extramakeopts))
if ec!=0:
    if not opt.quiet:
        print ("ERROR: Build problems encountered")
    sys.exit(1 if ec > 128 else ec)

if not opt.quiet:
    print (prefix)
    print ('%sSuccessfully built and installed all enabled packages!'%prefix)
    print (prefix)
    print ('%sSummary:'%prefix)
    print (prefix+'  Framework directory              : %s'%dirs.fmwkdir)
    print (prefix+'  Projects directory               : %s'%dirs.projdir)
    print (prefix+'  Installation directory           : %s'%dirs.installdir)
    print (prefix+'  Build directory                  : %s'%dirs.blddir)

    import col
    col_ok = col.ok
    col_bad = col.bad
    col_end = col.end
    #FIXME: Use formatlist module!
    def formatlist(lin,col):
        l=lin[:]
        colbegin = col if col else ''
        colend = col_end if col else ''
        if not l or l==['']: return '<none>'
        first=True
        out=''
        while l:
            s=''
            while l and len(s)<40:
                if s: s+=' '
                s+=l.pop(0)
            if first:
                out+='%s%s%s'%(colbegin,s,colend)
                first=False
            else:
                out += '\n%s                                     %s%s%s'%(prefix,colbegin,s,colend)
        return out
        return ' '.join(l)
    
    pkd_src_info = []
    for basedir in dirs.pkgsearchpath:
      pkg_nr = len([p.name for p in pkgloader.pkgs if p.dirname.startswith(str(basedir))])
      pkg_enabled = len([p.name for p in pkgloader.pkgs if p.enabled and p.dirname.startswith(str(basedir))])
      pkd_src_info.append([basedir, pkg_enabled, (pkg_nr-pkg_enabled) ])

    def pkg_info_str(info):
      return "%s (%s%d%s built, %s%d%s skipped)"%(info[0], col_ok,info[1],col_end, col_bad,info[2],col_end,)

    print (prefix+'  Package search path              : %s'%formatlist([pkg_info_str(info) for info in pkd_src_info],None))

    nmax = 20
    pkg_enabled = sorted([p.name for p in pkgloader.pkgs if p.enabled])
    n_enabled = len(pkg_enabled)
    lm2='(%i more,'
    limittxt=['...',lm2,'supply','--verbose','to','see','all)']
    if not opt.verbose and n_enabled>nmax:
        limittxt[1] = lm2%(n_enabled-nmax)
        pkg_enabled = pkg_enabled[0:nmax]+limittxt
    pkg_disabled = sorted([p.name for p in pkgloader.pkgs if not p.enabled])
    n_disabled = len(pkg_disabled)
    if not opt.verbose and n_disabled>nmax:
        limittxt[1] = lm2%(n_disabled-nmax)
        pkg_disabled = pkg_disabled[0:nmax]+limittxt
    import env
    extdeps_avail = sorted(k for (k,v) in env.env['extdeps'].items() if v['present'])
    extdeps_missing = sorted(k for (k,v) in env.env['extdeps'].items() if not v['present'])

    #Compilers (Fortran is considered optional), CMake, and required externals deps like Python and Boost:
    reqdep = [('CMake',env.env['system']['general']['cmake_version'])]
    for lang,info in env.env['system']['langs'].items():
        if not info:
            continue
        if info['cased']!='Fortran':
            reqdep += [(info['cased'],info['compiler_version_short'])]
            for dep in info['dep_versions'].split(';'):
                reqdep += [tuple(dep.split('##',1))]
        else:
            assert not info['dep_versions']#If allowed, we would need to print them somewhere

    print (prefix+'  System                           : %s'%env.env['system']['general']['system'])
    cp=env.env['cmake_printinfo']
    unused_vars = set(cp['unused_vars'])
    unused_vars_withvals=[]
    ucvlist = []
    for k,v in cfgvars.items():
        if k in unused_vars:
            ucvlist+=['%s%s=%s%s'%(col_bad,k,pipes.quote(v),col_end)]
            unused_vars_withvals+=[ucvlist[-1]]
        else:
            ucvlist+=['%s=%s'%(k,pipes.quote(v))]

    print (prefix+'  User configuration variables[*]  : %s'%formatlist(ucvlist,None))
    print (prefix+'  Required dependencies            : %s'%formatlist(['%s[%s]'%(k,v) for k,v in sorted(set(reqdep))],None))
    print (prefix+'  Optional dependencies present    : %s'%formatlist(['%s[%s]'%(e,env.env['extdeps'][e]['version']) for e in extdeps_avail],
                                                                       col_ok))
    print (prefix+'  Optional dependencies missing[*] : %s'%formatlist(extdeps_missing,col_bad))
    pkgtxt_en ='%s%i%s package%s built successfully'%(col_ok if n_enabled else '',
                                                      n_enabled,
                                                      col_end if n_enabled else '',
                                                      '' if n_enabled==1 else 's')
    pkgtxt_dis='%s%i%s package%s skipped due to [*]'%(col_bad if n_disabled else '',
                                                      n_disabled,
                                                      col_end if n_disabled else '',
                                                      '' if n_disabled==1 else 's')
    print (prefix+'  %s : %s'%(pkgtxt_en.ljust(32+(len(col_end)+len(col_ok) if n_enabled else 0)),formatlist(pkg_enabled,col_ok)))
    print (prefix+'  %s : %s'%(pkgtxt_dis.ljust(32+(len(col_end)+len(col_bad) if n_disabled else 0)),formatlist(pkg_disabled,col_bad)))
    print (prefix)
    if cp['unused_vars']:
        print (prefix+'%sWARNING%s Unused user cfg variables  : %s'%(col_bad,col_end,formatlist(unused_vars_withvals,None)))
        print (prefix)
    if cp['other_warnings']:
        print (prefix+'%sWARNING%s unspecified warnings from CMake encountered during environment inspection!'%(col_bad,col_end))
        print (prefix)

if opt.runtests:
    utils.rm_rf(dirs.testdir)
    ec=utils.system('. %s/setup.sh && dgtests --excerpts=%i -j%i --prefix "%s " --dir "%s"'%(dirs.installdir,opt.nexcerpts,opt.njobs,prefix,dirs.testdir))
    if ec==0 and (cp['unused_vars'] or cp['other_warnings']):
        print (prefix+'%sWARNING%s There were warnings (see above)'%(col_bad,col_end))
        print (prefix)

    if ec:
        sys.exit(1 if ec > 128 else ec)

if opt.install:
    ec=utils.system('. %s/setup.sh && dginstall -q "%s"'%(dirs.installdir,opt.install))
    if ec:
        sys.exit(1 if ec > 128 else ec)
    if not opt.quiet:
        print (prefix+'Copied self-contained installation to:')
        print (prefix)
        print (prefix+'  %s'%opt.install)
        print (prefix)

if not opt.quiet:
    if cfgvars.get('ONLY','')=='Framework::*' and not 'NOT' in cfgvars:
        import col
        print (prefix+"%sNote that only Framework/ packages were enabled by default:%s"%(col.bldmsg_notallselectwarn,col.end))
        print (prefix)
        print (prefix+"%s  - To enable pkgs for a given project do: dgbuild -p<projectname>%s"%(col.bldmsg_notallselectwarn,col.end))
        print (prefix+"%s  - To enable all pkgs do: dgbuild -a%s"%(col.bldmsg_notallselectwarn,col.end))
        print (prefix)
    print (prefix+"You are all set to begin using the software!")
    print (prefix)
    print (prefix+'To see available applications, type "ess_" and hit the TAB key twice.')
    print (prefix)
