#!/usr/bin/env python3
import subprocess
import pathlib
import os
import sys

#Replicated in Core/GitUtils.py:
def strbytes2path(strbytes_path):
    try:
        s = os.fsdecode(strbytes_path) if isinstance(strbytes_path,bytes) else strbytes_path
        fpath = pathlib.Path(s)
        str(fpath).encode('utf8')#check that it can be encoded in utf8 (TODO: Require ASCII?)
    except (UnicodeDecodeError,UnicodeEncodeError):
        raise SystemExit('Forbidden characters in filename detected! : "%s"'%fn)
    return fpath

#Replicated in Core/GitUtils.py:
#We set GIT_CONFIG=/dev/null to avoid user/system cfg messing with the
#result. We also set GIT_OPTIONAL_LOCKS=0 instead of using
#--no-optional-locks, since the functionality is not available in all git
#versions we want to support:
_safe_git_cmd = '/usr/bin/env GIT_CONFIG=/dev/null GIT_OPTIONAL_LOCKS=0 git'.split()

#Replicated in Core/GitUtils.py:
def git_status_iter(extra_args=[],staged_only=True,status_filter=''):
    cmd = _safe_git_cmd + ['diff','--name-status','--no-renames','-z']
    if staged_only:
        cmd += ['--cached']
    if status_filter:
        cmd += ['--diff-filter=%s'%status_filter]
    if extra_args:
        cmd += ['--']
        cmd += extra_args
    b=subprocess.run(cmd,check=True, stdout=subprocess.PIPE).stdout
    parts=b.split(b'\x00')
    while parts:
        statusflag=parts.pop(0).decode('ascii')
        if not parts:
            if not statusflag:
                break#guard against trailing empty entry
            raise RuntimeError('Unexpected output of git diff command (%s)'%(cmd))
        fpath = strbytes2path(parts.pop(0))
        yield statusflag,fpath

def query_git_version():
    gv = subprocess.run(['git','--version'],check=True,stdout=subprocess.PIPE).stdout
    return tuple(int(e) for e in gv.decode().split()[2].split('.')[0:2])

def get_next_githooks_dir():
    cmd='git config --get --global --path core.hooksPath'
    cp=subprocess.run(cmd.split(),
                      stdout=subprocess.PIPE)
    #return code of 1 can simply indicate that core.hooksPath is not set
    if cp.returncode not in (0,1):
        print('Warning: Problems running "%s" - no hook-chaining possible'%cmd)
        return None
    nextdir = cp.stdout if cp.returncode==0 else None
    if not nextdir and 'GIT_DIR' in os.environ:
        nextdir = strbytes2path(os.environ['GIT_DIR']).joinpath('hooks')
    elif nextdir:
        nextdir = strbytes2path(nextdir)
    if nextdir and nextdir.exists():
        return nextdir

def impl_pre__commit():
    cfg_size_lim_utf8 = 200*1024 # bytes
    cfg_size_lim_other = 20*1024 # bytes
    block_str = "nocommit".upper()#avoid actually writing the block string value in this file.

    #This checks directly file sizes and (indirectly) invalid characters in file-names.
    #It also looks for blocking string block_str added to the files.
    #
    #For future consideration:
    #      - check that there are no file-name clashes for case-insensitive
    #        systems (e.g. if both File.txt and file.txt exsists it could give
    #        trouble on OSX).
    #      - Check that text files end with a newline (and not too many)
    #      - Check against tabs in certain files.
    #      - For now we don't check GIT_AUTHOR_EMAIL or GIT_AUTHOR_NAME env vars.
    #
    cfg_size_lim_min = min(cfg_size_lim_other,cfg_size_lim_utf8)
    files_with_blockstr=[]
    status_filter='d'#ignore files that were deleted.
    if query_git_version() < (2,13):
        #older git (not sure how old, but at least 2.13 is new enough)
        #does not support the lowercase-means-exclude support on
        #--diff-filter, so for those we instead try to list all the
        #other options and hope that works:
        status_filter='ACMRTU'

    for flg_staged,fpath in git_status_iter(status_filter=status_filter):
        #Now, find the SHA id of the staged file (NOT the file in the
        #working-tree, as it might be unstaged or only partially staged):
        _=subprocess.run(_safe_git_cmd+['ls-files','--stage',str(fpath)],check=True, stdout=subprocess.PIPE).stdout
        objectid=_.split()[1].decode()
        #Extract content (all in memory is OK - we don't want people to commit very large objects anyway):
        objectdata = subprocess.run(_safe_git_cmd+['show',objectid],check=True, stdout=subprocess.PIPE).stdout
        #Determine if content is valid utf8:
        try:
            objectdata_str = objectdata.decode('utf8')
            is_utf8 = True
        except UnicodeDecodeError:
            is_utf8 = False
        #Implement file-size check:
        size_lim=(cfg_size_lim_utf8 if is_utf8 else cfg_size_lim_other)
        if len(objectdata) > size_lim:
            raise SystemExit(('Error: file-size limit of %g kB exceeded for %s file %s of size %i B'
                              '')%(size_lim/1024,('text' if is_utf8 else 'binary'),fpath,len(objectdata)))
        #Check against the block_str in text files:
        if is_utf8:
            for l in objectdata_str.splitlines():
                if block_str in l:
                    files_with_blockstr += [fpath]
                    break
    if files_with_blockstr:
        raise SystemExit(('Error: Forbidden string "%s" encountered in the files:\n  %s'
                          '')%(block_str,'\n  '.join(str(fp) for fp in sorted(files_with_blockstr))))

def githook_pre__commit():
    try:
        impl_pre__commit()
    except:
        print("commit aborted by dgcode checks",file=sys.stderr, flush=True)
        raise

def safe_rename_orig(origfile):
    return origfile.with_name(origfile.name+'_orig_renamed_by_dgcode')#NB: Must synchonise name with that found in dgcode/.system/install_local_githooks.py

def hook_dispatch():
    #Run the githook_xxx() function which corresponds to the filename with which
    #we were called.
    thisscript = pathlib.Path(__file__)
    hookname = thisscript.stem
    hooknamedescr = hookname
    hookfct=globals().get("githook_%s"%(hookname.replace('-','__')),None)
    if hookfct:
        print("calling %s hook"%hookname)
        hookfct()

    #Call the next hook in the path (workaround for the fact that git does not
    #support multiple hook directories, so we want to respect any global git
    #hooks the user has set up:
    if safe_rename_orig(thisscript).exists():
        nexthook = safe_rename_orig(thisscript)
        hooknamedescr='original %s'%hookname
    else:
        next_hooksdir = get_next_githooks_dir()
        nexthook = next_hooksdir.joinpath(hookname) if next_hooksdir else None
    if nexthook and nexthook.exists() and not nexthook.samefile(pathlib.Path(__file__)):
        hooks_receiving_data_on_stdin={'post-rewrite','pre-push','pre-receive','post-receive'}
        if hookname in hooks_receiving_data_on_stdin:
            print (("warning: ignoring %s hook from %s"
                    " (chaining of hooks reading stdin is not yet supported)")%(hooknamedescr,nexthook.parent))
        else:
            print("invoking %s hook from %s"%(hooknamedescr,nexthook.parent))
            sys.exit(subprocess.run([str(nexthook)]+sys.argv[1:]).returncode)


if __name__ == "__main__":
    hook_dispatch()


