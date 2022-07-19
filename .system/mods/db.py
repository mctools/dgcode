import os

#Database for remembering package info between invocations.

db = { 'pkg2timestamp' : {},#to detect when files changed
       'pkg2inc' : {},#keep track of provided public headers (populate at target __init__ stage)
       'pkg2parts' : {},#keep track of targets to the point where we can cleanup the install area when user removes files or dirs
       'pkg2reldir' : {},#keep track of pkg locations
       'pkg2runnables' : {},#Keep track of runnables (with cmdname, e.g. ess_bla_lala)
       'pkg2reflogs' : {},#Keep track of reference logs (e.g. ess_bla_lala.log)
   }

#initial load:
import dirs,utils
dbfile=dirs.blddir / 'globdb.pkl'

if os.path.exists(dbfile):#fixme: utils should have pkl_load safe which returns None if file does not exists (avoids potential race conds)
    db = utils.pkl_load(dbfile)

def save_to_file():
    utils.pkl_dump(db,dbfile)

def clear_pkg(pkgname):
    for k,v in db.items():
        if k.startswith('pkg2') and pkgname in v:
            del v[pkgname]
