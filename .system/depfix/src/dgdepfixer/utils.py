
from dgdepfixer.std import *
from dgdepfixer import allmods as DG

_cache = []
def _init():
    global _cache
    if not _cache:
        _allowed_swnames = list(sorted(modname[5:] for _,modname,_ in pkgutil.iter_modules(DG.__path__) if modname.startswith('inst_')))
        _mapcasings = dict((sw.lower(),sw) for sw in _allowed_swnames)
        _cache += [_allowed_swnames,_mapcasings]
    return _cache

def allowed_swnames():
    """Names of optional SW for --instsw=NAME"""
    return _init()[0]

def map_swname_casings(swname):
    #Map non-standard casings to correct ones, e.g. geant4->Geant4, nCrystal->NCrystal, Openscenegraph->OpenSceneGraph
    return _init()[1].get(swname.lower(),swname)

def swname_allowed(s):
    return map_swname_casings(s) in allowed_swnames()

def launch_dummy_cmake(content,rerun_wo_catching = True):
    """Launch dummy cmake project and return (output,customtxt_contents). In case of problems, a
       RuntimeError will be thrown. If rerun_wo_catching is True, problems will
       result in a rerun without output blocking, for easier inspection by
       users. The second return value is the content of a file custom.txt (if created, otherwise None)."""
    from dgdepfixer import swversions
    mincmake = swversions['CMake']['oldest']
    sys.stdout.flush()
    sys.stderr.flush()
    with tempfile.TemporaryDirectory() as tmpdir:
        td=pathlib.Path(tmpdir)
        preamble="""
cmake_minimum_required(VERSION %s)
project(DGDepFixerDummyCMake)
set(Python3_FIND_VIRTUALENV ONLY)#Never find python3 installation outside our virtualenv
"""%mincmake
        td.joinpath('CMakeLists.txt').write_text(preamble+content)
        cmd=lambda co: DG.sysutils.system_throw('cd %s && cmake .'%shlex.quote(str(td)),catch_output=co)
        try:
            out=cmd(True)
            customtxt = td.joinpath('custom.txt')
            return (out,customtxt.read_text() if customtxt.exists() else None)
        except RuntimeError:
            sys.stdout.flush()
            sys.stderr.flush()
        if rerun_wo_catching:
            cmd(False)#Again, to show output (could probably be improved with better usage of subprocess)
        raise RuntimeError#Make 100% sure we throw the second time

def extract_py3info_via_cmake(quiet=False):
    if not quiet:
        print("Launching CMake to extract information about Python3 setup")
    _,customtxt=launch_dummy_cmake("""
set(Python_ADDITIONAL_VERSIONS 3 3.15 3.14 3.13 3.12 3.11 3.10 3.9 3.8 3.7 3.6 3.5 )
set(Python3_USE_STATIC_LIBS FALSE)
find_package(Python3 3.5 REQUIRED COMPONENTS Interpreter Development)
if(PYTHON3_FOUND AND Python3_Interpreter_FOUND AND Python3_Development_FOUND )
  file(WRITE custom.txt "Found=1\n")
  foreach(v INTERPRETER_ID EXECUTABLE VERSION LIBRARIES INCLUDE_DIRS)
    file(APPEND custom.txt "${v}=${Python3_${v}}\n")
  endforeach()
else()
  file(WRITE custom.txt "Found=0\n")
endif()
""",True)
    info = dict(l.split('=',1) for l in customtxt.splitlines())
    #Sanity checks:
    if info['Found']!='1':
        raise RuntimeError('CMake could not find python3 (with interpreter+shared library+headers).')
    py3inpath=DG.sysutils.ResolvedAbsPath(shutil.which('python3'))
    py3fromcmake=DG.sysutils.AbsPath(info['EXECUTABLE'])
    if not py3fromcmake.exists():
        raise RuntimeError('CMake found python3 executable which does not exist: %s'%py3fromcmake)
    if not py3inpath.samefile(py3fromcmake):
        raise RuntimeError('CMake found python3 executable which is different from the one in the PATH')
    libs = info['LIBRARIES'].split(';')
    if len(libs)!=1:
        raise RuntimeError('CMake could not find exactly one python3 library to link against')
    incdirs = info['INCLUDE_DIRS'].split(';')
    if len(incdirs)!=1:
        raise RuntimeError('CMake could not find exactly one python3 include directory with development headers')
    return (str(py3inpath),libs[0],incdirs[0])


def find_scripts_with_bad_interpreter():
    """Looks in PATH and yields scripts whose shebang invokes a non-existing
       command, or calls it via /usr/bin/env. Be conservative and try to avoid
       false positives. The main objective is to locate scripts which were
       previously functioning, but who did not get cleanup when something was
       uninstalled.
    """
    def get_shebang(p):
        with p.open('rb') as fh:
            b=fh.read(1024*8)
            if b[0:2]==b'#!' and  b'\n' in b:
                return b[2:].split(b'\n',1)[0]

    usrbinenv = pathlib.Path('/usr/bin/env')
    pathdirs = [pathlib.Path(p.strip()).absolute().resolve() for p in os.environ.get('PATH','').split(':') if pathlib.Path(p.strip()).is_dir()]
    treated=set()
    for p in pathdirs:
        if p in treated:
            continue
        treated.add(p)
        if str(p) in ['/usr/bin','/bin','/usr/sbin','/sbin','/opt/X11/bin']:
            continue#Ignore these as a speedup, hoping most issues are in local
                     #pip installs, etc. (also: saw some unrelated positive hit in
                     #/opt/X11/bin/xkeystone)
        if str(p).startswith('/afs/cern.ch/atlas'):
            continue#several broken scripts in /afs/cern.ch/atlas at lxplus (not sure how to avoid injecting these, so just ignoring)
        for f in p.iterdir():
            if DG.sysutils.is_executable_file(f):
                try:
                    shebang = (get_shebang(f) or b'').decode()
                except UnicodeDecodeError:
                    continue
                shebang_parts = (shebang or '').split()
                if not shebang_parts:
                    continue
                p = pathlib.Path(shebang_parts[0])
                if p.exists() and usrbinenv.samefile(p) and len(shebang_parts)>1:
                    #shebang is of form '/usr/bin/env <cmd>'
                    p = pathlib.Path(shebang_parts[1])
                    if not shutil.which(str(p)):
                        #Missing command!
                        yield f.absolute(),' '.join(shebang_parts)
                else:
                    if not DG.sysutils.is_executable_file(p):
                        yield f.absolute(),' '.join(shebang_parts)

