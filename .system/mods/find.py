from __future__ import print_function
import os
import fnmatch
import col
import conf
from grep import pkgfiles,pkgdir_for_search
pjoin=os.path.join

def find(pkg,pattern):
    n=0
    #rd=pkgdir_for_search(pkg)
    rd=pkg.reldirname#always match on package reldir, even for dynamic packages!!
    pattern='*%s*'%pattern.lower()
    for flocal in pkgfiles(pkg):
        f = pjoin(rd,flocal)
        if fnmatch.fnmatch(f.lower(),pattern):
            ff=os.path.relpath(pjoin(pkg.dirname,flocal))
            if not pkg.isdynamicpkg or flocal==conf.package_cfg_file:
                print('match:',ff)
            else:
                ffreal=os.path.relpath(os.path.realpath(pjoin(pkgdir_for_search(pkg),flocal)))
                print('match in dynamic content: %s (real file is %s)'%(ff,ffreal))
            n+=1
    return n

#col.grep_match
