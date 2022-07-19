#!/usr/bin/env python3
import pathlib
import tempfile
import zipfile
import stat

def update(always=False):
    thisfile = pathlib.Path(__file__)
    thisdir  = thisfile.parent.absolute()
    srcdir   = thisdir.joinpath('src')
    target   = thisdir.joinpath('bld/bin/dgdepfixer')

    src_pymods = list(p for p in srcdir.joinpath('dgdepfixer').glob('*.py') if not '#' in p.name)#ignore files with '#' to ignore emacs backup files
    src_pymain = srcdir.joinpath('__main__.py')

    deps = src_pymods + [src_pymain,thisfile]
    if not always and target.exists() and target.stat().st_mtime > max(s.stat().st_mtime for s in deps):
        return

    target.parent.mkdir(parents=True,exist_ok=True)
    #ziplvl = zipfile.ZIP_DEFLATED#This would perform compression
    ziplvl = zipfile.ZIP_STORED#Do not actually compress anything (supposedly slightly more portable?)
    with tempfile.TemporaryDirectory() as tmpdirstr:
        tmpdir = pathlib.Path(tmpdirstr)
        tmpzip = tmpdir.joinpath('tmp.zip')
        with zipfile.ZipFile(str(tmpzip), 'w',ziplvl) as fh:
            fh.write(str(srcdir.joinpath('__main__.py')),'__main__.py')
            for f in src_pymods:
                fh.write(str(f),'dgdepfixer/%s'%f.name)
        cnt=b'#!/usr/bin/env python3\n'+tmpzip.read_bytes()
        target.write_bytes(cnt)
        if cnt!=target.read_bytes():
            raise SystemExit('Race condition in dgdepfixer compilation detected. Remove %s and rerun %s to be safe.'%(target,__file__))
        target.chmod(target.stat().st_mode | stat.S_IXOTH | stat.S_IXGRP | stat.S_IEXEC )

if __name__=='__main__':
    import sys
    update(always=('ALWAYS' in sys.argv[1:]))
