from pathlib import Path
import dgbuild.cfg as cfg

def extract_zlib_license():
    zlib_h=Path(cfg.pkgs['MiniZLib']['dirname']) / 'libinc/zlib.h'
    out = []
    for l in open(zlib_h):
        l=l.rstrip()
        out += [l]
        if 'madler@' in l and 'jloup@' in l:
            break
    out+=['*/']
    return out

#for l in extract_zlib_license():
#    print(l)
