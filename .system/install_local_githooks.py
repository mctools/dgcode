#!/usr/bin/env python3
import pathlib
import subprocess
import sys
import os

def _query_git_version():
    gv = subprocess.run(['git','--version'],check=True,stdout=subprocess.PIPE).stdout
    return tuple(int(e) for e in gv.decode().split()[2].split('.')[0:2])

the_git_version = _query_git_version()

def safe_rename_orig(origfile):
    return origfile.with_name(origfile.name+'_orig_renamed_by_dgcode')#NB: Must synchonise name with that found in <repos>/.githooks/hooks.py

def install_hooks( repo_root):
    #In principle a simple matter of "git config --local core.hooksPath
    #.githooks", but git earlier than v2.9 does not support this!
    hooksdir=repo_root / '.githooks'
    repo_root = hooksdir.parent
    assert repo_root.exists() and repo_root.joinpath('.git/HEAD').exists()
    if the_git_version >= (2,9):
        #"git config --local core.hooksPath .githooks" leads to various troubles
        #when CWD is outside the repo (and --git-dir does not always help). So
        #we simply perform a brute-force hack on .git/config to make sure the
        #last line with the word hooksPath in the file is the one we write.
        gitlocalcfg=repo_root.joinpath('.git/config')
        linetowrite='\n[core] hooksPath = .githooks\n'
        if not gitlocalcfg.exists():
            gitlocalcfg.write_text(linetowrite)
        else:
            lasthookspath=None
            for l in gitlocalcfg.read_text().splitlines():
                l=l.split('#',1)[0].strip()#discard comments
                if 'hooksPath' in l:
                    lasthookspath=l
            if not lasthookspath or lasthookspath.strip()!=linetowrite.strip():
                with gitlocalcfg.open("at") as fh:
                    fh.write(linetowrite)
    else:
        #Git is old, time for the brute-force approach of having to
        #put all of the hooks manually into .git/hooks. However, we only do:
        #
        #WARNING: THIS CODE BRANCH IS UNTESTED AFTER THE OCT-2022 MULTIREPO MIGRATION OF DGCODE!!
        srcdir = hooksdir
        targetdir = repo_root.joinpath('.git/hooks')
        if targetdir.exists() and not targetdir.is_dir():
            raise SystemExit('Error: .git/hooks exists but is not directory!')
        targetdir.mkdir(exist_ok=True)
        for f in hooksdir.glob('*'):
            if '~' in f.name:
                continue
            t=targetdir.joinpath(f.name)
            if t.exists():
                if not t.samefile(f):
                    t.rename(safe_rename_orig(t))
            if not t.exists():
                t.symlink_to(os.path.relpath(str(f),str(t.parent)))

def get_all_affected_git_repos( path ):
    l=[]
    for e in path.split(':'):
        if not e.strip():
            continue
        p = pathlib.Path(e).expanduser().absolute().resolve()
        if not p in l and p.joinpath('.git/HEAD').exists() and p.joinpath('.githooks/dgcode_auto_add_hookpath').exists():
            l.append( p )
    return l

def main():
    assert len(sys.argv) == 2
    for repo_root in get_all_affected_git_repos( sys.argv[1] ):
        print("CHECKING GIT HOOKS IN",repo_root)
        os.chdir(repo_root)#git is very sensitive about cwd!
        install_hooks( repo_root )

if __name__ == '__main__':
    main()
