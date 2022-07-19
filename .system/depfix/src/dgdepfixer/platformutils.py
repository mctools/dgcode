
from dgdepfixer.std import *
from dgdepfixer import allmods as DG

from dgdepfixer.sysutils import AbsPath,ResolvedAbsPath,SafeResolvedAbsPath

def osx_release():
    #detect osx release. python3 platform.release gives darwin release, eg. '19.3.0'
    #Map from https://en.wikipedia.org/wiki/Darwin_(operating_system):
    if platform.system() != 'Darwin':
        return None
    darwinmajornum_2_osx = { 20 : (11,  0, 'Big Sur'),#NB: Not sure this is entirely correct!
                             19 : (10, 15, 'Catalina'),
                             18 : (10, 14, 'Mojave'),
                             17 : (10, 13, 'High Sierra'),
                             16 : (10, 12, 'Sierra'),
                             15 : (10, 11, 'El Capitan'),
                             14 : (10, 10, 'Yosemite'),
                             13 : (10,  9, 'Mavericks'),
                             12 : (10,  8, 'Mountain Lion'),
                             11 : (10,  7, 'Lion'),
                             10 : (10,  6, 'Snow Leopard') }
    dmn = int(platform.release().split('.')[0])
    if dmn in darwinmajornum_2_osx:
        return darwinmajornum_2_osx[ dmn ]
    if dmn > 100 or dmn < 20:
        raise SystemExit('ERROR: Could not detect OSX version')
    return (11,dmn-20,'Unknown')#fwd compat - guess!

def _detect_extra_env_vars_needed():
    #only run once!
    #detect needed env. vars. Add them as needed even if already set (because
    #they might have been set only intermittently)
    out = {}
    if not pathlib.Path('/usr/include/stdlib.h').exists():
        osx_rel = osx_release()
        assert osx_rel is not None,"I dont know how to fix include files on this platform..."
        catalina_or_later =  osx_rel is not None and osx_rel[0:2]>=(10,15)
        if catalina_or_later:
            sdkroot=None
            if shutil.which('xcrun'):
                _=DG.sysutils.quote_cmd([shutil.which('xcrun'),'--show-sdk-path'])
                sdkroot = DG.sysutils.system_throw(_,catch_output=True).decode().strip()
            else:
                sdkroot='/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk'
            orig_sdkroot=os.environ.get('SDKROOT',None)
            if orig_sdkroot and pathlib.Path(orig_sdkroot).exists():
                #respect existing SDKROOT settings (the user might have set them consciously)
                sdkroot=orig_sdkroot
            if sdkroot and pathlib.Path(sdkroot).exists():
                assert not 'SDKROOT' in out
                out['SDKROOT'] = sdkroot
    return out

_extra_env_vars=[None]
def detect_and_set_extra_env_vars_needed():
    global _extra_env_vars
    if _extra_env_vars[0] is None:
        out = _detect_extra_env_vars_needed()
        for k,v in out.items():
            os.environ[k]=v
        _extra_env_vars[0]=out
    return _extra_env_vars[0]

def linux_dist_ids():
    """Return a list of identified linux distribution ID's, with the earlier entries
    in the list being more specific (similar to ID+ID_LIKE fields from
    /etc/os-release)"""
    distids=[]
    #First try to read os-release files, as per:
    #https://www.freedesktop.org/software/systemd/man/os-release.html
    f=pathlib.Path('/etc/os-release')
    if not f.exists():
        f = pathlib.Path('/usr/lib/os-release')
    if f.exists():
        id_like = []
        for l in (f.read_text().splitlines() if hasattr(f,'read_text') else open(str(f),'rt').read().splitlines()):
            parts = l.split('=',1)
            if len(parts)==2 and parts[0].strip()=='ID':
                distids += [parts[1].strip().lower()]
            if len(parts)==2 and parts[0].strip()=='ID_LIKE':
                id_like=parts[1].strip().lower().split()
        distids += id_like

    if not distids:
        #No info found so far, just fall back to the platform.dist function, if
        #available:
        if hasattr(platform,'dist'):
            distids += [platform.dist()[0].lower()]

    #make sure that mint linux is seen as a ubuntu derivative:
    if 'mint' in distids or 'linuxmint' in distids:
        distids += ['ubuntu']

    #Finally, increase probability to find either redhat or debian in the list (if appropriate):
    if 'ubuntu' in distids:
        distids += ['debian']
    elif set(distids).intersection({'fedora','rhel','centos'}):
        distids += ['redhat']
    else:
        #final resort, look for old-style files:
        if pathlib.Path('/etc/redhat-release').exists():
            distids += ['redhat']
        elif pathlib.Path('/etc/debian_version').exists():
            distids += ['debian']

    #remove duplicates and return:
    used,r=set(),[]
    for e in distids:
        if not e in used:
            used.add(e)
            r += [e]
    return r

_inspect_platform_cache=[{}]
def inspect_platform():
    """Inspect platform and return dictionary with relevant info. Throws SystemExit in case of fundamental issues with a completely unsupported platform."""
    if _inspect_platform_cache[0]:
        return _inspect_platform_cache[0]

    sysname = platform.system().lower()
    if sysname not in ('linux','darwin'):
        raise SystemExit('ERROR: Unsupported platform ("%s")'%sysname)#TODO: Support FreeBSD?

    if not pathlib.Path('/bin/bash').exists():
        raise SystemExit('ERROR: Could not find BASH shell at /bin/bash. Your platform is likely completely unsupported!')

    if not pathlib.Path('/usr/bin/env').exists():
        raise SystemExit('ERROR: Could not find the /usr/bin/env command. Your platform is likely completely unsupported!')

    #Proceed to investigate missing SW and suggest ways to install it.
    is_linux = sysname=='linux'
    is_osx = not is_linux

    _linux_dist_ids = linux_dist_ids() if is_linux else []
    is_rhlike = 'redhat' in _linux_dist_ids
    is_debianlike = is_linux and not is_rhlike and 'debian' in _linux_dist_ids

    has_homebrew = is_osx and shutil.which('brew')#TODO: homebrew now supports linux as well (but rather recently here in 2019)
    installcmd = None
    if is_linux:
        for p in (['dnf','yum','apt-get'] if not is_debianlike else ['apt-get','dnf','yum']):
            if shutil.which(p):
                installcmd = 'sudo %s install %%s'%p
                break
    elif has_homebrew:
        installcmd = 'brew install %s'

    _inspect_platform_cache[0].update(dict( sysname = sysname,
                                            is_linux = is_linux,
                                            is_osx = is_osx,
                                            is_rhlike = is_rhlike,
                                            is_debianlike = is_debianlike,
                                            has_homebrew = has_homebrew,
                                            installcmd = installcmd ) )
    return _inspect_platform_cache[0]

def has_ok_cmake():
    try:
        v = DG.sysutils.system_throw('cmake --version',True).split()[2]
    except RuntimeError:
        v = None
    return not version_issue('CMake',v)

_cache_venvwithpip=[None]
def can_do_venv_with_pip():
    global _cache_venvwithpip
    if _cache_venvwithpip[0] is None:
        with tempfile.TemporaryDirectory() as tempdir:
            d=AbsPath(tempdir).joinpath('testvenv')
            _cache_venvwithpip[0] = True
            try:
                DG.sysutils.system_throw('python3 -mvenv  %s'%shlex.quote(str(d)),True)
                DG.sysutils.system_throw('. %s && python3 -mpip --version'%shlex.quote(str(d.joinpath('bin/activate'))),True)
            except RuntimeError:
                _cache_venvwithpip[0] = False
    return _cache_venvwithpip[0]

def base_check(do_git=True,do_pipvenv=True,do_py3dev=True):
    """Detect issues (and suggest fixes) for issues which is otherwise outside the
    scope of the present script to fix relating to python3 version+headers, C++
    compiler, BASH, git. Prints messages to stderr and raise SystemExit."""

    if os.environ.get('GITHUB_SERVER_URL',''): #Ignore git config when using GitHub Runners (CI)
      do_git=False

    #############################
    ### Sanity check DGDEPDIR ###
    #############################

    dgdepdir_envvar = os.environ.get('DGDEPDIR',None)
    if dgdepdir_envvar and not AbsPath(dgdepdir_envvar).exists():
        raise SystemExit('Environment variable DGDEPDIR is set to a path which does not exist!\n       You should probably unset it (type "unset DGDEPDIR") and rerun.')

    dgdepdir = ResolvedAbsPath(dgdepdir_envvar) if dgdepdir_envvar else None
    if dgdepdir and not dgdepdir.joinpath('.dgcode_dependencies_installation_done').exists():
        errprint('Environment variable DGDEPDIR points to an incomplete installation (%s)!'%dgdepdir)
        if dgdepdir.joinpath('unsetup.sh').exists():
            errprint('You should probably try to disable it by typing:')
            errprint('    . %s/unsetup.sh'%dgdepdir)
        else:
            errprint('You should probably unset the variable (type "unset DGDEPDIR")')
        errprint('It might also be a good idea to manually remove the directory at %s'%dgdepdir)
        errprint('Afterwards, you can try to rerun the command you are currently running')
        raise SystemExit(1)

    #########################################################################################################################
    ### Detect platform info (throws SystemExit in case of very fundamental issues, e.g. on Windows or without /bin/bash) ###
    #########################################################################################################################
    platforminfo = inspect_platform()
    installcmd = platforminfo['installcmd']


    ##############################################################################################
    ## Make sure that user's default shell is BASH and that ~/.bashrc is always sourced somehow ##
    ##############################################################################################

    defshell = SafeResolvedAbsPath(os.environ.get('SHELL','/bin/sh'))
    shell = defshell.name
    if shell=='sh':
        #perhaps /bin/sh is not a symlink?
        try:
            if 'bash' in DG.sysutils.system_throw('%s --version'%defshell,catch_output=True):
                shell = 'bash'
        except RuntimeError:
            pass

    if shell!='bash':
        #likely some power users are fiddling with the default shell... or OSX Catalina introduce zsh as new default (argh).
        errprint('ERROR: Your default SHELL appears to be %s which is untested and unsupported. The only supported'%shell)
        errprint('       shell is BASH so if you wish to proceed, you have to change it (alternatively bypass/fool')
        errprint('       this check by setting SHELL=/bin/bash).')
        if platforminfo['is_osx'] and pathlib.Path('/bin/bash').exists():
            errprint('')
            errprint('       You can most likely change this by entering the command "chsh -s /bin/bash" in a terminal.')
            errprint('       Afterwards you must close and reopen the current terminal for the change to take effect.')
        raise SystemExit(1)

    #Now make sure that ~/.bashrc is always sourced somehow
    has_bashrc = AbsPath('~/.bashrc').exists()

    if os.environ.get('GITHUB_SERVER_URL',''): #Ignore when using GitHub Runners (CI)
      print('GitHub Runner environment detected. Skipping bash profile file checking.')
    else:
      candidates = [n for n in list(AbsPath(m) for m in ('~/.bash_profile','~/.bash_login','~/.profile')) if n.exists()]
      if not candidates and has_bashrc:
          errprint('You appear to be running BASH with a ~/.bashrc file, but no profile file is sourcing it.')
          errprint('\nTherefore it will not always be sourced in your terminal sessions.')
          errprint('\nYou can fix this by running the following command:\n')
          errprint("     echo 'if [ -f $HOME/.bashrc ]; then . $HOME/.bashrc; fi' >> $HOME/.bash_profile")
          errprint('\nOf course you should first check that the ~/.bashrc file does not contain anything harmful! (You should check that anyway!)')
          raise SystemExit(1)
      if len(candidates)>1:
          errprint('You appear to be running BASH with several profile files present: %s'%(' '.join(str(c) for c in candidates)))
          errprint('Only the %s file is actually used in this case and the other ones are ignored.'%candidates[0])
          errprint('To avoid confusion, you should remove the unused ones. For instance by running:\n')
          for c in candidates[1:]:
              errprint('    mv %s %s_DISABLED'%(c,c))
          errprint()
          raise SystemExit(1)
      if not candidates and not has_bashrc:
          errprint('You appear to be running BASH with no profile files and no ~/.bashrc file.')
          errprint('\nYou should create them by running the following command:\n')
          errprint("     echo 'if [ -f $HOME/.bashrc ]; then . $HOME/.bashrc; fi' >> $HOME/.bash_profile && touch $HOME/.bashrc")
          raise SystemExit(1)
      assert len(candidates)==1
      proffile = candidates[0]
      if not has_bashrc:
          errprint('You appear to be running BASH with no ~/.bashrc file. You should create one by running:')
          errprint("\n     touch $HOME/.bashrc")
          raise SystemExit(1)
      if not any('.bashrc' in l.split('#')[0] for l in proffile.read_text().splitlines()):
          errprint('You appear to be running BASH with a ~/.bashrc file, but your profile file (%s) does not source it!'%proffile)
          errprint('You can fix this by running the following command:\n')
          errprint("     echo 'if [ -f $HOME/.bashrc ]; then . $HOME/.bashrc; fi' >> %s"%proffile)
          errprint('\nOf course you should first check that the ~/.bashrc file does not contain anything harmful! (You should check that anyway!)')
          raise SystemExit(1)

    osx_rel = osx_release()
    catalina_or_later =  osx_rel is not None and osx_rel[0:2]>=(10,15)

    if catalina_or_later and not os.environ.get('BASH_SILENCE_DEPRECATION_WARNING',None):
        print()
        print('Notice: In case you are getting annoying warnings from OSX about zsh not being the default')
        print('        shell (whenever you open a new terminal), you can try to disable them by running')
        print('        this command:   echo "export BASH_SILENCE_DEPRECATION_WARNING=1" >> ~/.bashrc')
        print()


    ################################################################################
    ## Search system for broken scripts, referring to obsolete installations, etc. #
    ################################################################################

    shebangissues=collections.defaultdict(list)
    for f,shebang in DG.utils.find_scripts_with_bad_interpreter():
        if f.name=='c_rehash' and 'perl' in shebang:
            #known bug, unlikely to be due to user's system being particularly broken:
            #https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=281212
            continue
        if f.name.endswith('.csh'):
            #ignore .csh scripts installed with ROOT/Geant4
            continue
        shebangissues[shebang] += [f]
    if shebangissues:
        if os.environ.get('DGDEP_IGNOREBADSHEBANG','')=='1':
            n=sum(len(files) for  _,files in shebangissues.items())
            print('WARNING: Ignoring %i broken scripts since DGDEP_IGNOREBADSHEBANG is set.'%n)
        else:
            errprint()
            for shebang,files in shebangissues.items():
                errprint('Bad shebang: %i files refer to a defunctional command ("%s"):'%(len(files),shebang))
                for f in files:
                    errprint('   %s'%f)
            errprint()
            errprint('ERROR: Broken scripts found in active PATH. This is often a symptom of broken installations')
            errprint('       of SW which was left over when something was only partly installed. You should probably')
            errprint('       fix this, as it can give rise to all sorts of hard-to-debug issues with your system.')
            errprint('       It is hard to give an exact recipe for what to do, but one course of action would of')
            errprint('       course be to remove the files listed above, or at the very least remove their directories')
            errprint('       from your PATH variable (perhaps added in your ~/.bashrc?), or make them non-executable')
            errprint('       by running "chmod -x <filename>". It is probably a good idea to ally yourself with an')
            errprint('       expert if you are not sure what to do. For instance ask %s'%DG.__email__)
            errprint()
            errprint('       If you would like to simply ignore these broken scripts (not usually recommended), then')
            errprint('       you can set DGDEP_IGNOREBADSHEBANG=1 (make permanent by adding "export DGDEP_IGNOREBADSHEBANG=1"')
            errprint('       to your ~/.bashrc file) and rerun.')
            raise SystemExit(1)


    ################################################
    ## Check for presence of Make and C++ compiler #
    ################################################

    cppcmds=['c++','g++','gcc','clang++','icc']
    lacks_cpp = not any(shutil.which(cmd) for cmd in cppcmds + [os.environ.get('CC','c++')])
    lacks_make = not shutil.which('make')
    lacks_git = do_git and not shutil.which('git')

    if platforminfo['is_osx'] and (lacks_cpp or lacks_make or lacks_git):
        errprint('ERROR: Missing C++ compiler and/or "make" command for makefile based building and/or "git" command.')
        errprint('You probably have to:')
        errprint('    1) install XCode (from the AppStore)')
        errprint('    2) run this command in a terminal: "sudo xcode-select --install"')
        raise SystemExit(1)

    if lacks_make:
        errprint('ERROR: Missing  "make" command for makefile based building.')
        cmd=None
        if installcmd:
            if platforminfo['is_rhlike']:
                cmd = installcmd%'make'
            elif platforminfo['is_debianlike']:
                cmd = installcmd%'build-essential'
        if cmd:
            errprint('You might be able to install them with a command like: "%s"'%cmd)
        elif platforminfo['is_linux']:
            errprint('You might have to install a package named build-essential, make, or similar')
        raise SystemExit(1)

    if lacks_cpp:
        errprint('No C++ compiler detected in PATH (looked for: %s, and the $CC environment variable)'%(', '.join(cppcmds)))
        cmd=None
        if installcmd:
            if platforminfo['is_rhlike']:
                cmd = installcmd%'gcc-c++'
            elif platforminfo['is_debianlike']:
                cmd = installcmd%'g++'
        if cmd:
            errprint('You might be able to install them with a command like: "%s"'%cmd)
        elif platforminfo['is_linux']:
            errprint('You might have to install a package named g++, gcc-c++, or similar')
        raise SystemExit(1)

    ########################################################################################
    ## On OSX, check that /usr/include is populated (it is not by default from OSX 10.14) ##
    ########################################################################################

    is_mojave = osx_rel and ( osx_rel[-1]=='Mojave' )

    if is_mojave and platforminfo['is_osx'] and not pathlib.Path('/usr/include/stdlib.h').exists():
        pf=pathlib.Path('/Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg')
        if not pf.exists():
            l=list(pf.parent.glob('macOS_SDK_headers_for_macOS_*.pkg'))
            if len(l)==1:
                pf=l[0]
        errprint('Could not find standard system C-headers in /usr/include')
        errprint('')
        errprint('This is likely because you are running OSX Mojave (10.14) or later and Apple decided to "helpfully" remove them.')
        if pf.exists():
            errprint('')
            errprint('You might be able to install the missing headers with a command like:')
            errprint('')
            errprint('  sudo installer -pkg %s -target /'%pf)
            errprint('')
            errprint("or (if it doesn't help), try this command:")
            errprint('')
            errprint('  open %s'%pf)
            errprint('')
        else:
            errprint('')
            errprint("Unfortunately you do not have a file like %s, so I don't know how to help you..."%pf)
            errprint('')
        sys.exit(1)


    ###########################################################################################
    ## Make sure python3 has a consistent installation of dev headers+library+python3-config ##
    ###########################################################################################

    if do_py3dev:
        try:
            import distutils.sysconfig as dusc
        except ImportError:
            dusc = None

        pyincdir = dusc.get_python_inc() if dusc else None
        if not pyincdir or not AbsPath(pyincdir).joinpath('pyconfig.h').exists():
            if dusc:
                errprint('ERROR: Could not find python3 development headers.')
            else:
                errprint('ERROR: Could not import distutils.sysconfig module.')
            cmd=None
            if installcmd:
                if platforminfo['is_rhlike']:
                    cmd = installcmd%'python3-devel'
                elif platforminfo['is_debianlike']:
                    cmd = installcmd%'python3-dev'
                elif platforminfo['has_homebrew']:
                    cmd = installcmd%'python3'
            if cmd:
                errprint('\nYou might be able to complete your installation with a command like: "%s"'%cmd)
            elif platforminfo['is_linux']:
                errprint('\nYou might have to install a package named python3-devel, python3-dev, or similar in order to complete your installation')
            raise SystemExit(1)

        pycfgcmd = shutil.which('python3-config')
        if pycfgcmd:
            for inc in set(DG.sysutils.system_throw('python3-config --includes',True).split()):
                if inc.startswith(b'-I'):
                    try:
                        inc=inc[2:].decode(sys.getfilesystemencoding())
                    except UnicodeDecodeError:
                        continue
                    if not AbsPath(pyincdir).samefile(AbsPath(inc)):
                        errprint( "ERROR: Inconsistency detected in python3 installation: "
                                  + "directories reported by distutils.sysconfig.get_python_inc"
                                  + " and python3-config --includes differ" )
                        raise SystemExit(1)
        if has_ok_cmake():
            #If has ok CMake, check that it detects Python3 properly (do nothing
            #here in case CMake is not ok, since it will likely be fixed later):
            try:
                _,_,cmake_py3incdir = DG.utils.extract_py3info_via_cmake(quiet=True)
            except RuntimeError as e:
                raise SystemExit("CMake is present and in recent version - but can not detect python3 properly (Reason: %s)"%(str(e) or 'unknown'))
            if not AbsPath(pyincdir).samefile(AbsPath(cmake_py3incdir)):
                errprint('ERROR: Inconsistency detected in python3 installation: ')
                errprint('       include directory found by CMake (%s) differs from'%cmake_py3incdir)
                errprint('       that reported by distutils.sysconfig.get_python_inc (%s)'%pyincdir)
                raise SystemExit(1)


    ########################################################
    ## Make sure python3 venv is not broken and check pip ##
    ########################################################

    if do_pipvenv:
        #Note: fedora with python3-devel has py3+venv+pip both inside and outside venv's
        #      ubuntu with python3-dev lacks pip (python3-pip) and venv's need python3-venv pkg to provide pip
        #      miniconda (tested on osx) has pip, but inside venv's pip is not made available.

        #Thus, we generally check only for basic venv --without-pip
        #functionaility. On debian-like systems, if sys.executable is
        #/usr/bin/python3, we furthermore check to see if the extra packages
        #needs to be installed.

        with tempfile.TemporaryDirectory() as tempdir:
            cmd='python3 -mvenv --without-pip  %s'%shlex.quote(str(AbsPath(tempdir).joinpath('dotest_venv_nopip')))
            ec,out=DG.sysutils.system(cmd,True)
            if ec!=0:
                errprint( '\n\nERROR: Incomplete python3 installation detected: Can not use venv module ("python3 -mvenv --without-pip <dir>" fails) !\n\n' )
                errprint( "Output of command \"%s\" was:\n"%cmd)
                errprint(out)
                raise SystemExit(1)

            syspy3=SafeResolvedAbsPath('/usr/bin/python3')
            is_like_stdubuntu_with_stdpy3 = platforminfo['is_debianlike'] and installcmd and syspy3 and syspy3.samefile(AbsPath(sys.executable))
            if is_like_stdubuntu_with_stdpy3 and not can_do_venv_with_pip():
                errprint( "\n\nERROR: Incomplete python3 installation detected: Can only use venv module with \"--without-pip\" flag!\n\n" )
                errprint( "Output of command \"%s\" was:\n"%cmd)
                errprint(out)
                errprint('\nPerhaps you need to run "%s" to complete your installation'%(installcmd%'python3-venv python3-pip')
                         +' (otherwise, perhaps the output above contained some hints?).\n')
                raise SystemExit(1)

    if do_git:
        if lacks_git:
            errprint('\n\nERROR: No "git" command found')
            if installcmd:
                errprint("You can try to install via the command:   %s"%(installcmd%'git'))
        gitissue = version_issue('git',probe_git_version())
        if gitissue:
            errprint('Git %s!'%gitissue)
        else:
            try:
                _un = DG.sysutils.system_throw("git config user.name",True).strip().decode('ascii')
                _ue = DG.sysutils.system_throw("git config user.email",True).strip().decode('ascii')
                if not '@' in _ue or not '.' in _ue or not _un or not _ue:
                    raise RuntimeError
            except UnicodeDecodeError:
                errprint('ERROR: "git config user.name" or "git config user.email" command returned non-ASCII characters.\n')
                errprint('You should fix the entry in your git config file (usually ~/.gitconfig).\n')
                raise SystemExit(1)
            except RuntimeError:
                errprint('ERROR: "git config user.name" or "git config user.email" command failed or did not give appropriate information.\n')
                errprint('You should make sure your git config file (usually ~/.gitconfig) contains an appropriate entry in the following format:')
                errprint("""
[user]
  name = James Chadwick
  email = james.chadwick@ess.eu
""")
                raise SystemExit(1)
            if any('jameschadwick' in _.lower().replace(' ','').replace('.','') for _ in (_ue,_un)):
                errprint('ERROR: "git config user.name" or "git config user.email" command indicates you are James Chadwick. That seems unlikely...\n')
                errprint('You should fix the entry in your git config file (usually ~/.gitconfig).\n')
                raise SystemExit(1)

def probe_git_version():
    try:
        return DG.sysutils.system_throw('git --version',True).split(b' version ')[1].split()[0].decode('ascii')
    except RuntimeError:
        return None

def probe_system_versions():
    """Probe system for installed versions of dependencies listed in the swversions dictionary"""
    sysinfo = {}
    #CMake:
    try:
        _ = DG.sysutils.system_throw('cmake --version',True).split()[2]
        sysinfo['CMake'] = _.decode('ascii')
    except RuntimeError:
        sysinfo['CMake'] = None
    #Git:
    sysinfo['git'] = probe_git_version()
    #Python modules:
    for depkey,pymodname in sorted((k,k[6:]) for k in DG.swversions.keys() if k.startswith('pymod:')):
        try:
            _=DG.sysutils.system_throw('python3 -c "import %s as m;print(m.__version__)"'%pymodname,True).strip()
            sysinfo[depkey] = _.decode('ascii')
        except RuntimeError:
            sysinfo[depkey] = None
        if sysinfo[depkey] is None and pymodname=='ptpython':
            #Workaround for ptpython which does not define __version__. If we can import it, we fake the version as the recommended one.
            try:
                DG.sysutils.system_throw('python3 -c "import %s as m"'%pymodname,True).strip()
            except RuntimeError:
                pass
            else:
                sysinfo[depkey] = DG.swversions[depkey]['recommended']

    #Sanity check that we checked the correct dependencies:
    l0,l1=sorted(list(DG.swversions.keys())),sorted(list(sysinfo.keys()))
    assert l0==l1,"\n%s ==\n%s"%(l0,l1)
    #Check that all detected versions can be converted to tuples:
    for v in sysinfo.values():
        v and version_tuple(v)
    return sysinfo

def version_tuple(s):
    """Accepts versions in formats like b'1.2.3', '1.2.3', (1,2,3) and returns a tuple like
    (1,2,3). Patch number will be set to 0 if not specified (e.g. '2.3' becomes (2,3,0))
    For robustness, patch versions are allowed to contain letters but any content after them
    will be ignored: '1.2.3rc6'->(1,2,3) and '1.2.alpha3' -> (1,2,0). For robustness, versions
    like "2.8.12.2" will ignore the sub-patchlevel number and be converted to (2,8,12).
    """
    #bytes -> str:
    s = s.decode('utf-8') if isinstance(s,bytes) else s
    #str -> list of parts:
    if isinstance(s,str):
        s = list(e for e in s.split('.'))
        if len(s)==3:
            #handle letters in patch version:
            a = [x.isalpha() for x in s[2]]
            s[2]= (s[2][0:a.index(True)] or '0') if any(a) else s[2]
    v = tuple(int(e) for e in s[0:3])#ignore any parts after the third entry

    assert len(v) in (2,3),"unexpected version: %s / %s"%(v,s)
    return (v[0],v[1],0) if len(v)==2 else v

def version_issue(depkey,version):
    """returns None in case of no issues, otherwise returns a non-empty string explaining the issue. If passed in version is None, it is supposed to be missing"""
    if version is None:
        return 'is missing (recommended version is %s)'%DG.swversions[depkey]['recommended']
    v = version_tuple(version)
    newest = DG.swversions[depkey].get('newest',None)
    oldest = DG.swversions[depkey]['oldest'] or DG.swversions[depkey]['recommended']#oldest defaults to recommended
    if oldest and (v<version_tuple(oldest)):
        return 'is too old (%s < %s)'%(version,oldest)
    if newest and (v>version_tuple(newest)):
        return 'is too new (%s < %s)'%(version,oldest)

def _check_and_advice_common(name,pkgnames,extralines=None,findpkgline=True):
    pinfo = inspect_platform()
    ctnt='\n'.join((['find_package(%s REQUIRED)'%name] if findpkgline else [])+(extralines or []))
    try:
        DG.utils.launch_dummy_cmake(ctnt)
        return
    except RuntimeError:
        pass
    errprint()
    errprint("-----------------------------------------------------------------------------")
    errprint("ERROR: %s not found!"%name)
    instcmd=pinfo['installcmd']
    fullinstcmd = None
    if instcmd:
        debian_pkg = pkgnames.get('debian',None)
        rh_pkg = pkgnames.get('redhat',None)
        brew_pkg = pkgnames.get('brew',None)
        for systype,sysname in [('is_debianlike','debian'),
                                ('is_rhlike','redhat'),
                                ('has_homebrew','brew')]:
            pkgname = pkgnames.get(sysname,None) if pinfo[systype] else None
            if pkgname:
                fullinstcmd = instcmd%pkgname
                break
    if fullinstcmd:
        errprint("       You might be able to fix your system by running a command like:")
        errprint("       %s"%fullinstcmd)
    else:
        errprint("       You should make sure it is installed on your system in a location accessible to CMake.")
    errprint("-----------------------------------------------------------------------------")
    errprint()
    raise SystemExit(1)

def check_and_advice_expat():
    _check_and_advice_common('EXPAT',dict(debian='libexpat1-dev     (or try libexpat-dev instead)',
                                          redhat='expat-devel',
                                          brew='expat'))


def check_and_advice_xercesc():
    _check_and_advice_common('XercesC',dict(debian='libxerces-c-dev',
                                            redhat='xerces-c-devel',
                                            brew='xerces-c'))

def check_and_advice_x11subpkg(pkgname):
    extralines="""
find_package(X11 REQUIRED)
if ( NOT X11_%s_FOUND)
    message( FATAL_ERROR "X11_%s not found" )
endif()
"""%(pkgname,pkgname)
    _check_and_advice_common('X11_%s'%pkgname,
                             dict(debian='lib%s-dev'%pkgname.lower(),
                                  redhat='lib%s-devel'%pkgname),#TODO not sure where libxpm/libxft comes from in homebrew (bundled with xquartz?)
                             extralines=extralines.splitlines(),
                             findpkgline=False)
