
from dgdepfixer.std import *
#from dgdepfixer import allmods as DG

#quote from shlex + make pathlib-aware:
from shlex import quote as _shlex_quote
quote = lambda s : _shlex_quote(str(s) if hasattr(s,'__fspath__') else s)

def chmod_x(path):
    """python equivalent of 'chmod +x'. This version always sets +x for all of user, group, others."""
    path.chmod(path.stat().st_mode | stat.S_IXOTH | stat.S_IXGRP | stat.S_IEXEC )

def mkdir_p(path,*args):
    """python equivalent of 'mkdir -p ...'"""
    if args:
        mkdir_p(path)
        for a in args:
            mkdir_p(a)
        return
    pathlib.Path(path).mkdir(parents=True, exist_ok=True)

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
    if not hasattr(path,'is_dir'):
        return isemptydir(pathlib.Path(path))
    return path.is_dir() and not any(True for _ in path.iterdir())

def is_executable_file(p):
    """returns True if path is to a file which user can execute (i.e. it must
       exist, be readable and executable, and not be a directory).
    """
    try:
        return p.exists() and not p.is_dir() and os.access(str(p.absolute()), os.X_OK|os.R_OK)
    except PermissionError:
        return False

def quote_cmd(cmd):
    if isinstance(cmd,list):
        return ' '.join(quote(c) for c in cmd)
    return cmd

def system(cmd,catch_output=False):
    """A better alternative to os.system which flushes stdout/stderr, makes sure the
       shell is always bash, and wraps exit codes larger than 127 to 127. Set
       catch_output to True to instead return both exit code and the output of the
       command in a string."""
    #flush output, to avoid confusing ordering in log-files:
    sys.stdout.flush()
    sys.stderr.flush()
    #rather than os.system, we call "bash -c <cmd>" explicitly through
    #the subprocess module, making sure we can always use bash syntax:
    cmd=['bash','-c',cmd]
    #wrap exit code to 0..127, in case the return code is passed on to
    #sys.exit(ec):
    fixec = lambda ec : ec if (ec>=0 and ec<=127) else 127
    if catch_output:
        try:
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            output = p.communicate()[0]
        except BaseException:
            #todo: in case of failure we should return the output as well!
            return 1,''
        return fixec(p.returncode),output
    else:
        ec=subprocess.call(cmd)
    sys.stdout.flush()
    sys.stderr.flush()
    return fixec(ec)

def system_throw(cmd,catch_output=False):
    """same as system except doesn't return exit code and throws RuntimeError in case of non-zero exit code"""
    out=None
    if catch_output:
        ec,out = system(cmd,catch_output)
    else:
        ec = system(cmd,catch_output)
    if ec:
        raise RuntimeError('command failed: %s'%cmd)
    return out

_urlopen_args=[None]
def _debug_http_and_ssl(print_prefix):
    testurl_nossl = 'http://google.com'
    testurl_withssl = 'https://google.com'
    #Test connection (no ssl)
    urlopen = urllib.request.urlopen
    print('%sTesting internet connection - trying to access %s'%(print_prefix,testurl_nossl))
    try:
        #with urlopen(testurl_nossl,cafile='/etc/ssl/certs/Sonera_Class_2_Root_CA.pem') as resp:
        with urlopen(testurl_nossl) as resp:
            pass
    except urllib.error.URLError as e:
        errprint('%s ERROR: Connection failed! (%s)\n\n\n'%(print_prefix,str(e)))
        raise SystemExit('Internet connection failed')#NB: This raise command was missing, I hoped I used the correct exception type.
    #Test ssl:
    print('%sTesting Python SSL setup - trying to access %s'%(print_prefix,testurl_withssl))
    bad_ssl_cert=False
    try:
        with urlopen(testurl_withssl) as resp:
            pass
    except urllib.error.URLError as e:
        if 'CERTIFICATE_VERIFY_FAILED' in str(e):
            bad_ssl_cert=True
        else:
            raise#unknown, better pass up
    if not bad_ssl_cert:
        _urlopen_args[0] = {}
        return

    print('%sWARNING: Problems with Python SSL setup detected - will try to look for certificates.'%print_prefix)
    _f1 = pathlib.Path('/etc/ssl/certs.pem')
    _f2 = pathlib.Path('/etc/ssl/cert.pem')
    _d = pathlib.Path('/etc/ssl/certs')
    if _f1.exists():
        _urlopen_args[0]=dict(cafile=str(_f1))
    elif _f2.exists():
        _urlopen_args[0]=dict(cafile=str(_f2))
    elif _d.exists():
        _urlopen_args[0]=dict(capath=str(_d))
    if _urlopen_args[0] is not None:
        #cafile/capath deprecated, use context instead:
        _urlopen_args[0]=dict(context=ssl.create_default_context(ssl.Purpose.SERVER_AUTH,**_urlopen_args[0]))
        print('%sRetrying access to %s using certificates from /etc/ssl'%(print_prefix,testurl_withssl))
        try:
            with urlopen(testurl_withssl,**_urlopen_args[0]) as resp:
                pass
            bad_ssl_cert=False
        except urllib.error.URLError as e:
            pass
        if bad_ssl_cert:
            print('%sWARNING: SSL setup could not be fixed with certificates in /etc/ssl.'%print_prefix)
            _urlopen_args[0]=None
    if bad_ssl_cert:
        #NB: Could check 'import certifi; cafile=cerfifi.where() as well if
        #needed to have another workaround. We could also look at SSL env vars.
        raise SystemExit('Unrecoverable problems with Python SSL setup detected.')
    print('%sCertificates in /etc/ssl seems to be working.'%print_prefix)
    assert _urlopen_args[0] is not None

def ensure_debug_http_and_ssl( print_prefix = '' ):
    global _urlopen_args
    if _urlopen_args[0] is None:
        _debug_http_and_ssl(print_prefix)
        assert not _urlopen_args[0] is None

def urlretrieve(url, filename, reporthook=None, data=None):
    """Like urllib.request.urlretrive, but that was marked deprecated so
    here is a simplified version with less features, and which use our SSL fix."""
    with contextlib.closing(urllib.request.urlopen(url, data, **_urlopen_args[0])) as fp:
        headers = fp.info()
        with open(filename, 'wb') as outfile:
            result = filename, headers
            bs,size,read,blocknum = 1024*8,-1,0,0
            if "content-length" in headers:
                size = int(headers["Content-Length"])
            if reporthook:
                reporthook(blocknum, bs, size)
            while True:
                block = fp.read(bs)
                if not block:
                    break
                read += len(block)
                outfile.write(block)
                blocknum += 1
                if reporthook:
                    reporthook(blocknum, bs, size)
    if size >= 0 and read < size:
        raise urllib.error.ContentTooShortError(
            "retrieval incomplete: got only %i out of %i bytes"
            % (read, size), result)
    return result

def download(url, filename, quiet = False, prefix=''):
    filename = str(filename) if isinstance(filename,pathlib.Path) else filename
    start_time=time.time()
    ensure_debug_http_and_ssl(prefix)#does nothing if already triggered
    def reporthook(count, block_size, total_size):
        if quiet:
            return
        duration = time.time() - start_time
        progress_size = count * block_size
        speed = round( progress_size / (1024. * duration))
        percent = max(0.0,min(100.0,(count * block_size * 100.0 / total_size)))
        min(int(count*block_size*100/total_size),100)
        s="%.1f%%, %.1f MB, %d KB/s, %ds" %(percent, progress_size / (1024. * 1024), speed, duration)
        print("\r%sDownloading %s [%s]    \r"%(prefix,url,s),end='')
    if not quiet:
        print()
    try:
        reporthook(0,1,1e99)
        urlretrieve(url, filename, reporthook)
    except urllib.error.HTTPError as e:
        print("%s !!! Problems downloading %s\n"%(prefix,url))
        raise e

#AbsPath: Return absolute path object from existing or non-existing path : Avoid Path.absolute() calls, as discussed on:
#  https://stackoverflow.com/questions/42513056/how-to-get-absolute-path-of-a-pathlib-path-object
#  https://bugs.python.org/issue29688
if sys.version_info[0:2]<=(3,5):
    #NB: We actually don't support python 3.5 anymore so could remove this code
    AbsPath = lambda p: pathlib.Path(os.path.abspath(str(pathlib.Path(p).expanduser())))
    ResolvedAbsPath = lambda p: AbsPath(p).resolve()
else:
    AbsPath = lambda p: pathlib.Path(p).expanduser().resolve(strict=False)
    ResolvedAbsPath = lambda p: pathlib.Path(p).expanduser().resolve(strict=True)
def SafeResolvedAbsPath(p):
    """ResolvedAbsPath(p) if exists, otherwise just AbsPath(p)"""
    ap = AbsPath(p)
    return ap.resolve() if ap.exists() else ap
AbsPathOrNone = lambda p : (AbsPath(p) if p else None)
def ResolvedAbsPathOrNone(p):
    """Returns None if p is None or a non-existing path. Otherwise returns the resolved path."""
    if p is None:
        return None
    _=AbsPath(p)
    if not _.exists():
        return None
    return _.resolve(strict=True)
