
#Before anything else, check the python version to completely avoid dealing with python 3.4 and older:
import sys
if sys.version_info[0:2]<(3,6):
    raise SystemExit("Unsupported Python version encountered. The Python version must be at least 3.6.")

#Is the setup working - can we import from the dgdepfixer package - use an empty module to avoid unrelated import errors?
try:
    import dgdepfixer
except ImportError:
    print('Setup ERROR: Could not import modules from the embedded dgdepfixer Python package - something is broken!')
    sys.exit(1)

#import all std modules here for early warning:
try:
    import dgdepfixer.std
except ImportError as e:
    print('Setup ERROR: Could not import standard python3 module. System is broken (%s)'%e)
    sys.exit(1)

#import the rest:
from dgdepfixer import allmods as DG

if __name__=='__main__':
    #Fix apparent name of command when run from unpacked sources as python3 src/__main__.py

    osbasename = dgdepfixer.std.os.path.basename
    if osbasename(sys.argv[0])=='__main__.py':
        sys.argv[0] = 'dgdepfixer'

    #Make sure we are always running as dgdepfixer (otherwise printouts referring back to ourselves will be confusin)
    if osbasename(sys.argv[0])!='dgdepfixer':
        raise SystemExit('ERROR: Must be launched with the name "dgdepfixer". Please rename and rerun')

    #Fix env vars (like SDKROOT)
    DG.platformutils.detect_and_set_extra_env_vars_needed()

    #Special mode for installing optional SW detected:
    if len(sys.argv)>1 and sys.argv[1].startswith('--instsw'):
        #Decode swname and absorp --instsw into sys.argv[0]:
        if sys.argv[1].startswith('--instsw='):
            swname = sys.argv[1][9:]
        elif len(sys.argv)>2:
            swname = sys.argv[2]
            del sys.argv[2]
        else:
            swname=''
        swname = DG.utils.map_swname_casings(swname)#Allow Openscenegraph->OpenSceneGraph, geant4->Geant4, etc.
        del sys.argv[1]
        sys.argv[0] = '%s --instsw=%s'%(osbasename(sys.argv[0]),swname)
        #Import and execute SW-specific command:
        allowed_swnames = DG.utils.allowed_swnames()
        if not swname in allowed_swnames:
            raise SystemExit('ERROR: Invalid SW name "%s" specified. Valid choices are: %s'%(swname,' '.join(allowed_swnames)))
        swmod = dgdepfixer.std.importlib.import_module('dgdepfixer.inst_%s'%swname)
        swmod.main()
    else:
        #Standard tool mode:
        if any(a.startswith('--instsw') for a in sys.argv[1:]):
            raise SystemExit('ERROR: --instsw can only be specified as the first argument')
        try:
            with dgdepfixer.std.tempfile.TemporaryDirectory() as tmpdir:
                DG.stdtool.main(DG.sysutils.AbsPath(tmpdir))
        except Exception as e:
            print((('-'*80)+'\n')*4+'\n')
            print('                       ABORTING DUE TO ERRORS!!\n')
            print((('-'*80)+'\n')*4+'\n')
            raise e
        except KeyboardInterrupt:
            print('Aborted by user...')

#TODO: It would be great if we could leave behind a log-file for debugging purposes...
#TODO: python3 -mcompileall would be useful at lxplus/dmsc where users don't have write access to the installation.
#TODO: If the system issues are only due to pip module versions (e.g. we
#      increment recommended numpy version), then the exit code from diagnose
#      should indicate that, and we should have a dgdepfixer --updatepyenv
#      mode. Thus avoiding rebuilding e.g. Geant4 and ROOT and downloading cmake. Or perhaps just recommend a pip update command?
#TODO: Avoid too harsh compatibility checks in platform-specific code on lxplus/dmsc? To avoid having to constantly recompile everything? Nahh... better eat own dogfood and make system less annoying.


#FINAL SPRINT:
#Geant4 must suggest how to fix XercesC issue (i.e. sudo apt-get lala)!
#dgdepfixer_install setup.sh/unsetup.sh must remind about unused dgdepfixer_install scripts
#check all fixmes/todos
#update dmsc+lxplus
#verify osx without brew/xcode...?
#update wiki
#Change default to python3 DGCODE_USEPY2 var
#Add py2 test to dgskynet
#Add py2 tests for MCPL + NCrystal (only things where we keep python2 support)


#GITMIGRATION Plan:
#   Before migration day:
#   a) DONE finalise dgdepfixer v5 (Change versions of some externals? Like cmake, numpy, matplotlib, ...)
#   b) DONE Update installations at dmsc + lxplus (in new _v0.0.5) dir, make dgcodefuture_git/bootstrap.sh use it.
#   On the migration day:
#   1) upload new dgdepfixer to lxplus/www, block other user access to the hg-dgcode repo (or just rename it?).
#   1) DONE rename ~/dgdepfixer_install_GITMIGRATION in dgcodefuture_git (in dgdepfixer and in bootstrap.sh)
#   2) checkout hg-dg_dgcode, close down access to hg-dg_dgcode, convert to git,
#       pull from it into dgcodefuture_git, rebase hg2git branch, push back to new local git, put into new git repo at bitbucket, fix permissions, etc.
#   3) mv old repo to obsolete version, push to new repo
#   4) update wiki (CodingFramework, HowtoInstall, ... , Mercurial, Git, ...)
#   5) send out announcement email
#   6) fix dgskynet
#   7) send out emails, make blog post.
