from __future__ import print_function

import os
import errno
import sys
import re
import conf
import shutil

def err(msg):
    if isinstance(msg,list):
        print ("ERROR: %s"%('\nERROR: '.join(msg)))
    else:
        print ("ERROR: %s"%msg)
    sys.exit(1)

def system(cmd):
    #flush output, to avoid confusing ordering in log-files:
    sys.stdout.flush()
    sys.stderr.flush()
    #rather than os.system, we call "bash -c <cmd>" explicitly through
    #the subprocess module, making sure we can always use bash syntax:
    import subprocess
    cmd=['bash','-c',cmd]
    try:
        ec=subprocess.call(cmd)
    except KeyboardInterrupt:
        sys.stdout.flush()
        sys.stderr.flush()
        import time
        time.sleep(0.2)
        print ("<<<Command execution interrupted by user!>>>")
        if hasattr(sys,'exc_clear'):
            sys.exc_clear()#python2 only
        ec=126
    #wrap exit code to 0..127, in case the return code is passed on to sys.exit(ec).
    sys.stdout.flush()
    sys.stderr.flush()
    if ec>=0 and ec<=127: return ec
    return 127


def run(cmd):
    sys.stdout.flush()
    sys.stderr.flush()
    if not isinstance(cmd,list):
        cmd=cmd.split()
    import subprocess
    output=None
    try:
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output = p.communicate()[0]
    except:
        return (1,'')
        pass
    return (p.returncode,output)

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

def rm_f(path):
    try:
        os.remove(path)#only succeeds if path exists and is not a directory
    except OSError as e:
        if e.errno == errno.ENOENT:#no such file or directory
            return#not an error, rm -f should exit silently here
        raise

def rm_rf(path):
    #First attempt with os.remove, and only in case it was a directory go for
    #shutil.rmtree (since shutil.rmtree prints error messages if fed symlinks to
    #directories):
    try:
        os.remove(path)
    except OSError as e:
        if e.errno == errno.ENOENT:#no such file or directory
            return#not an error, rm -rf should exit silently here
        elif e.errno != errno.EISDIR and e.errno!=errno.EPERM:
            raise
    #is apparently a directory
    try:
        shutil.rmtree(path)
    except OSError as e:
        if e.errno == errno.ENOENT:
            return#Sudden disappearance is still ok.
        raise

def isemptydir(path):
    if not os.path.isdir(path):
        return False
    for f in os.listdir(path):
        return False
    return True

def rmdir(path):
    if isemptydir(path):
        os.rmdir(path)

_normalre=re.compile('').match
def listfiles(d,filterfnc=0,error_on_no_match=True,ignore_logs=False):
    for f in os.listdir(d):
        if ignore_logs and f.endswith('.log'):
            continue
        if not conf.ignore_file(f) and not os.path.isdir(os.path.join(d,f)):
            if filterfnc:
                if filterfnc(f):
                    yield f
                elif error_on_no_match:
                    error.error("Forbidden file %s/%s"%(d,f))
            else:
                yield f

def is_executable(fn):
    return os.access(fn,os.X_OK)

#python3's pickle is python2's cPickle:
try:
    import cPickle as _pklmod
except ImportError:
    import pickle as _pklmod

try:
    from pathlib import Path as _Path
except ImportError:
    #python2 workaround:
    class _Path:
        def __init__(self,fn): self._fn=fn
        def open(self,mode): return open(self._fn,mode)

def pkl_load(fn_or_fh):
    if hasattr(fn_or_fh,'read'):
        #argument is filehandle:
        return _pklmod.load(fn_or_fh)#filehandle already
    else:
        #argument is filename:
        with _Path(fn_or_fh).open('rb') as fh:
            d=_pklmod.load(fh)
        return d

def pkl_dump(data,fn_or_fh):
    if hasattr(fn_or_fh,'write'):
        #argument is filehandle:
        d=_pklmod.dump(data,fn_or_fh)
        fn_or_fh.flush()
        return d
    else:
        #argument is filename:
        with _Path(fn_or_fh).open('wb') as fh:
            d=_pklmod.dump(data,fh)
        return d

def update_pkl_if_changed(pklcont,filename):
    old=os.path.exists(filename)
    changed = True
    if old:
        try:
            oldcont=pkl_load(filename)
        except EOFError:
            print ( "WARNING: Old pickle file %s ended unexpectedly"%filename)
            oldcont=(None,'bad....')
            pass
        if oldcont==pklcont:
            changed=False
    if old and changed:
        os.rename(filename,str(filename)+'.old')
    if changed:
        pkl_dump(pklcont,filename)

def touch(fname, times=None):
    #like unix touch, but creates directory if needed:
    mkdir_p(os.path.dirname(fname))
    with open(fname, 'a'):
        os.utime(fname, times)

def shlex_split(s):
    try:
        import shlex
    except ImportError:
        shlex=None
    if shlex:
        return shlex.split(s)
    else:
        return s.split()#hope user didnt use spaces in directories

_import_modatpath_count = [0]
def import_modatpath(pathtomodule,modulename=None):
    global _import_modatpath_count
    try:
        import importlib
        from importlib.util import spec_from_file_location,module_from_spec
    except ImportError:
        spec_from_file_location,module_from_spec=None,None
    #If not specified, construct a unique module name:
    if not modulename:
        _import_modatpath_count[0] = +1
        modulename = 'modatpathno%i'%(_import_modatpath_count[0])
    if spec_from_file_location and module_from_spec:
        #python3:
        spec = importlib.util.spec_from_file_location(modulename,pathtomodule)
        themodule = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(themodule)#don't catch exceptions here, want to propagate errors from embedded code
        return themodule
    else:
        #python2:
        import imp
        return imp.load_source(modulename,pathtomodule)
