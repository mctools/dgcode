import os
import dirs
import conf
import utils
import env

def recreate():
    fn=dirs.blddir / 'setup.sh'

    extra_binpath=':'.join(env.env['system']['runtime']['extra_bin_path'])
    extra_libpath=':'.join(env.env['system']['runtime']['extra_lib_path'])
    if extra_binpath: extra_binpath+=":"
    if extra_libpath: extra_libpath+=":"

    t="""#!/bin/bash

###############################################################################################################
# Autogenerated file which clients can source in order to use the installed software
#
DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ "x$<pn>_INSTALL_PREFIX" != "x$DIR" ]; then

    if [ ! -z "$<pn>_INSTALL_PREFIX" ]; then
        ############################################################################
        # Undo effect of sourcing setup file in different location
        function prunepath() {
            P=$(IFS=:;for p in ${!1:-}; do [[ $p != ${2}* ]] && echo -n ":$p"; done)
            export $1="${P:1:99999}"
        }
        prunepath PATH $<pn>_INSTALL_PREFIX
        prunepath PYTHONPATH $<pn>_INSTALL_PREFIX
    fi

  ###############################################################################################################
  # Set environment variables which clients can incorporate in their setup (Makefiles etc.):
  export <pn>_INSTALL_PREFIX="$DIR"
  export <pn>_INCLUDE_DIR="${<pn>_INSTALL_PREFIX}/include"
  export <pn>_LIB_DIR="${<pn>_INSTALL_PREFIX}/lib"
  export <pn>_DATA_DIR="${<pn>_INSTALL_PREFIX}/data"
  export <pn>_TESTREF_DIR="${<pn>_INSTALL_PREFIX}/tests/testref"

  ###############################################################################################################
  # Modify global path variables for our executables, scripts, libraries, python modules, etc. to be available:
  export PATH="${<pn>_INSTALL_PREFIX}/sysbin:${<pn>_INSTALL_PREFIX}/bin:${<pn>_INSTALL_PREFIX}/scripts:%s$PATH"
  export PYTHONPATH="${<pn>_INSTALL_PREFIX}/python:$PYTHONPATH"

fi
"""
    t=(t%extra_binpath).replace('<pn>',conf.projectname)
    #only write file if contents have changed
    fh=open(fn) if os.path.exists(fn) else None
    if fh and t==fh.read():
        fh.close()
        return
    if fh:
        fh.close()
    fh=open(fn,'w')
    fh.write(t)
    fh.close()
