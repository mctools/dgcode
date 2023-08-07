#!/bin/bash

#Special bootstrap file for developers wishing to build the framework without a
#project, and without needing to edit a local extraconf.sh file (the "projects"
#dir will be generated in the temp area).

#IMPORTANT: This file is version controlled, and should not be edited by users
#to avoid accidental pushing of personal settings to the repository. If you
#want to change configurations from the defauls in this file, please create a
#script named 'bootstrap_dev_extraconf.sh' next to it, and make the changes there.

export DGCODE_FMWK_DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export DGCODE_PROJECTS_DIR="/tmp/$USER/"$(cd "$DGCODE_FMWK_DIR"; python3 -c 'import pathlib; print(abs(hash(str(pathlib.Path(".").absolute().resolve()))),end="")')
if [ ! -f $DGCODE_PROJECTS_DIR ]; then
    mkdir -p "$DGCODE_PROJECTS_DIR"
    echo "$DGCODE_FMWK_DIR" > "$DGCODE_PROJECTS_DIR/dgcode_framework.txt"
fi
export DGCODE_BUILD_DIR="auto"
export DGCODE_INSTALL_DIR="auto"
export DGCODE_EXTRA_PKG_PATH=""
export DGCODE_ENABLE_PROJECTS_PKG_SELECTION_FLAG=false
if [ -f "$DGCODE_FMWK_DIR"/bootstrap_dev_extraconf.sh ]; then
    source "$DGCODE_FMWK_DIR"/bootstrap_dev_extraconf.sh
fi
. "$DGCODE_FMWK_DIR"/bootstrap.sh
