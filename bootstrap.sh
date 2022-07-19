#!/bin/bash

if [ "x$DGCODE_USEPY2" == "x1" ]; then
    echo "ERROR: You have set DGCODE_USEPY2=1, but Python2 support is now completely gone."
    echo 'Please "unset DGCODE_USEPY2" and source bootstrap.sh once again to try with Python3.'
    return 1
fi

if [[ -z "${DGCODE_PROJECTS_DIR}" ]]; then
  echo "ERROR: The DGCODE_PROJECTS_DIR variable must be set before sourcing bootstrap.sh"
  return 1
fi

function _dgcode_prunepath() {
    P=$(IFS=:;for p in ${!1}; do [[ $p != ${2}* ]] && echo -n ":$p"; done)
    export $1=${P:1:99999}
}

for tmp in DGCODE_DIR ESS_INSTALL_PREFIX; do
    #Cleanout leftover from previous invocations:
    if [ "x${!tmp}" != x ]; then
        _dgcode_prunepath PYTHONPATH "${!tmp}"
        _dgcode_prunepath PATH "${!tmp}"
        _dgcode_prunepath LD_LIBRARY_PATH "${!tmp}"
        _dgcode_prunepath DYLD_LIBRARY_PATH "${!tmp}"
        unset ${tmp}
    fi
done

#For safety, a rehash doesn't hurt:
hash -r > /dev/null 2>&1

#Where are we?:
DGCODE_DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ ! -x $DGCODE_DIR/.system/bin/dgbuild ]; then
    echo "Error: Could not find the dgbuild script"
    unset DGCODE_DIR
    return 1
fi

if  [[ "$DGCODE_DIR" == /afs/* ]] || [[ "$DGCODE_DIR" == /nfs/* ]] ; then
    #Prevent caching of python bytecode in .pyc/.pyo/__pycache__ when working on
    #network dir. Trying this as a solution after repeatedly encountering broken
    #.pyc files on AFS at CERN lxplus.
    export PYTHONDONTWRITEBYTECODE=1
fi

if [ -f $HOME/.dgcode_prebootstrap.sh ]; then
    source $HOME/.dgcode_prebootstrap.sh
fi

if [ "x$DG_DONT_SET_PYTHONUNBUFFERED" != x1 ]; then
    #make sure all print statements from python are unbuffered, otherwise they
    #will get interspersed with C++/C printouts in context-dependent order in
    #python3:
    export PYTHONUNBUFFERED=1
fi

#Detect clusters with special support:
DG_PLATFORM=
if [[ -d /mnt/groupdata ]] && [[ "x$HOSTNAME" = *.esss.dk ]]; then
    DG_PLATFORM=$( grep 'release 8' /etc/centos-release >/dev/null && echo dmsc8 || echo dmsc7 )
elif [[ -d /afs ]] && [[ "x$HOSTNAME" = *.cern.ch ]] && [[ -f /etc/centos-release ]]; then
    DG_PLATFORM=$( grep 'release 8' /etc/centos-release >/dev/null && echo lxplus8 || echo lxplus7 )
fi
if [ "x$DG_IGNORE_PLATFORM" == x1 ]; then
    DG_PLATFORM=
fi

if [ "x$DG_PLATFORM" != "x" ]; then
    echo
    echo "Detected platform with special support: $DG_PLATFORM"
fi

if [[ "x$DG_PLATFORM" == xdmsc7 ]]; then
    #unload all modules, get gcc 8.20, python3.5, a venv with common
    #modules, new cmake, etc:
    . /mnt/groupdata/detector/dgcode_installs/centos7_gcc820_py36_dgdepfixer_v0.1.0/setup.sh
    #Load this one here (nb: hdf5/1.10.6_gcc820 is broken as per 2020-03-23).
    module load hdf5/1.10.2_gcc540
fi
if [[ "x$DG_PLATFORM" == xdmsc8 ]]; then
    #echo "WARNING: DMSC centos8 platform support is on the way in the near future..."
    . /mnt/groupdata/detector/dgcode_installs/centos8_dgdepfixer_v0.1.0/setup.sh
fi

if [[ "x$DG_PLATFORM" == xlxplus7 ]]; then
    #Get gcc 8.20 + dgdepfixer installation:
    . /afs/cern.ch/work/t/tkittel/public/centos_devtools8_dgdepfixer_v1.0.0/setup.sh
fi

if [[ "x$DG_PLATFORM" == xlxplus8 ]]; then
    #Get dgdepfixer installation:
    . /afs/cern.ch/work/t/tkittel/public/centos8_dgdepfixer_v0.1.0/setup.sh
fi


#If we are not on a standard platform, try to look for SCL's/modules on CentOS
#to enable modern GCC/Python:
if [ "x$DGCODE_NOCENTOSHELP" == "x" -a "x$DG_PLATFORM" == x -a -f /etc/redhat-release ]; then
    echo "Checking if system is CentOS/RHEL in need of activation of SCL's or modules (can be disabled with 'export DGCODE_NOCENTOSHELP=1')"
    . $DGCODE_DIR/.system/setup_centos.sh
else
    unset DG_PLATFORM
fi

which python3 > /dev/null 2>&1
if [ $? != 0 ]; then
    echo "ERROR: You have no python3 installed."
    return 2
fi

/usr/bin/env python3 -c '' > /dev/null 2>&1
if [ $? != 0 ]; then
    echo "ERROR: You seem to have issues such as looping python3 symlinks in your path (\"/usr/bin/env python3\" fails). Some installation is broken!"
    ( IFS=:; for i in $PATH ; do if [ -d $i ]; then readlink -f $i/python3 >/dev/null || echo "NB: Circular broken python3 link found in: $i"; fi; done )
    return 2
fi

#Remove dgdepfixer
function _dgcode_resetdgdepfixerpath() {
    #Make sure all directories with dgdepfixer inside them are removed from PATH
    #(it is thus least intrusive if dgdepfixer is always alone in it's
    #directory). Also adds DGCODE_DIR/.system/depfix/bld/bin to the PATH:
    hash -r > /dev/null 2>&1
    while true; do
        DG_TMP_RDFP=`which dgdepfixer 2>&1` || DG_TMP_RDFP=notfound
        if [ "x$DG_TMP_RDFP" == xnotfound ]; then
            break
        fi
        _dgcode_prunepath PATH "$( cd -P "$( dirname "${DG_TMP_RDFP}" )" && pwd )"
    done
    unset DG_TMP_RDFP
    export PATH="$DGCODE_DIR/.system/depfix/bld/bin:$PATH"
}


#Make sure "compiled" dgdepfixer is up to date:
python3 "$DGCODE_DIR"/.system/depfix/compile.py || echo "ERRORS detected during dgdepfixer compilation!!!!"
#Inject it into the path and remove other dgdepfixer's:
_dgcode_resetdgdepfixerpath

#Default DGDEPDIR to ~/dgdepfixer_install unless user already sourced another:
DG_TMP="$HOME/dgdepfixer_install"
if [ "x$DGDEPDIR" != "x" ]; then
    DG_TMP="$DGDEPDIR"
fi
if [ -f "$DG_TMP/setup.sh" ]; then
    dgdepfixer --checkcompat "$DG_TMP"
    if [ $? == 0 ]; then
        if [ "x$DGDEPDIR" != "x" ]; then
            echo
            echo "-> Confirmed compatibility of active dgdepfixer installation at $DGDEPDIR"
            echo
        else
            echo
            echo "-> Activating installation at ~/dgdepfixer_install/"
            echo
            source $HOME/dgdepfixer_install/setup.sh
            #Remove the newly introduced (not-updated) dgdepfixer from PATH:
            _dgcode_resetdgdepfixerpath
        fi
    else
        if [ "x$DGDEPDIR" != "x" ]; then
            #FIXME: We should have a dgdepfixer --cheapupgradeexisting: leave extras, update (unsetup.sh), dgdepfixer, pymods (via pip calls), cmake (if needed)
            echo "WARNING: Currently active dgdepfixer installation at $DGDEPDIR is incompatible. Please run dgdepfixer and try to recreate it."
            return 2
        else
            echo
            echo "WARNING: Ignoring incompatible or incomplete installation in ~/dgdepfixer_install/ (you should run dgdepfixer to investigate)"
            echo
        fi
    fi
fi

#Sanity:
_dgcode_resetdgdepfixerpath

#cleanup:
unset _dgcode_resetdgdepfixerpath
unset _dgcode_prunepath

#diagnose dependencies:
dgdepfixer --diagnose
DG_TMP=$?
if [ $DG_TMP == 99 ]; then
    unset DG_TMP
    if [ "x$DGCODE_NODEPFIX" != "x1" ]; then
        if [ "x$DGDEPDIR" != "x" -a -f "$DGDEPDIR/unsetup.sh" ]; then
            echo
            echo "-> Deactivating installation at $DGDEPDIR"
            echo
            . "$DGDEPDIR/unsetup.sh"
        fi
        echo
        echo "WARNING: System issues detected. You can try to fix them now with the command:"
        echo
        echo "   dgdepfixer"
        echo
        echo "Alternatively: if you are an expert and think the detected issues are not important, you can ignore this check"
        echo "by 'export DGCODE_NODEPFIX=1'. In any case, you must source the bootstrap.sh file again once done:"
        echo
        echo "  . $DGCODE_DIR/bootstrap.sh"
        echo
        return 2
    else
        echo "Proceeding despite detected problems since DGCODE_NODEPFIX is set."
    fi
elif [ $DG_TMP != 0 ]; then
    echo "ERROR: Aborting setup due to fundamental platform issues"
    unset DG_TMP
    return 2
fi
unset DG_TMP

#setup local git hooks if relevant:
if [ -d "$DGCODE_DIR/.git" -a -d "$DGCODE_DIR/.githooks" ]; then
    #Make sure the hooks in .githooks directory are used to detect e.g. filesize
    #violations immediately upon commit:
    $DGCODE_DIR/.githooks/hooks.py --install-hooks-mode
fi


export DGCODE_DIR


#Resolve the location of the install and build directories. 
#Both default to the DGCODE_PROJECTS_DIR, but can be overridden by user
#specified DGCODE_INSTALL_DIR and DGCODE_BUILD_DIR, unless they're set to 'auto'
DGCODE_INSTALL_DIR_RESOLVED=$DGCODE_PROJECTS_DIR/install
if [[ -n "${DGCODE_INSTALL_DIR}" ]] && [ "$DGCODE_INSTALL_DIR" != "auto" ]; then
  DGCODE_INSTALL_DIR_RESOLVED=$DGCODE_INSTALL_DIR
fi
export DGCODE_INSTALL_DIR_RESOLVED

DGCODE_BUILD_DIR_RESOLVED=$DGCODE_PROJECTS_DIR/.bld
if [[ -n "${DGCODE_BUILD_DIR}" ]] && [ "$DGCODE_BUILD_DIR" != "auto" ]; then
  DGCODE_BUILD_DIR_RESOLVED=$DGCODE_BUILD_DIR
fi
export DGCODE_BUILD_DIR_RESOLVED


#Provide dgbuild and dgrun functions. Make sure the installation area's setup
#script is automatically sourced after each dgbuild invocation:

function dgbuild() {
    python3 $DGCODE_DIR/.system/bin/dgbuild "$@" ; dgec=$?; if [ -f $DGCODE_INSTALL_DIR_RESOLVED/setup.sh ]; then source $DGCODE_INSTALL_DIR_RESOLVED/setup.sh; fi; return $dgec
}

function dgrun() {
    if [ "x$#" == "x0" ]; then
        echo "Usage:"
        echo
        echo "dgrun <program> [args]"
        echo
        echo "Runs \"dgbuild > /dev/null\" and if it finishes successfully it"
        echo "proceeds to launch <program> within the dgcode runtime environment"
        return 1
    fi
    prog=$1
    dgbuild -q > /dev/null
    EC=$?
    if [ $EC != 0 ]; then
        echo "Build failed. Aborting"
        return $EC
    fi
    shift 1
    source $DGCODE_DIR/install/setup.sh && $prog "$@"
}

if [ -f "$DGCODE_INSTALL_DIR_RESOLVED/setup.sh" ]; then
    source "$DGCODE_INSTALL_DIR_RESOLVED/setup.sh"
fi

if [ "x$DGCODE_BOOTSTRAP_QUIET" != "x1" ]; then
    echo
    echo "Successfully bootstrapped the ESS Detector group code in: $DGCODE_DIR"
    echo
    echo "You now have the command 'dgbuild' available which you can run from any"
    echo "directory below $DGCODE_DIR to build the software."
    echo
    echo "For more info run: dgbuild --help"
    echo
fi
