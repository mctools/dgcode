#!/bin/bash

if [ -f /etc/os-release ]; then
    _tmpdetect=$(. /etc/os-release 2>/dev/null && echo x$ID ||echo xunknown)
    if [ "${_tmpdetect}" == xrhel -o "${_tmpdetect}" == xcentos ]; then
        _tmpdetect=$(echo $(gcc -dumpversion 2>/dev/null|| echo "0")|head -c1)
        if [ $_tmpdetect -le 7 ]; then
            echo "Centos/RHEL with missing or obsolete gcc detected."
            #Missing gcc or gcc is too old. Look for devtoolset SCL's. Prefer known to hypothetical versions.
            for _tmpdetect in devtoolset-8 devtoolset-7 devtoolset-9 devtoolset-10 devtoolset-11; do
                if [ -f /etc/scl/prefixes/$_tmpdetect ]; then
                    echo "  => Enabling $_tmpdetect SCL for modern gcc."
                    source scl_source enable $_tmpdetect
                    _tmpdetect=foundviascl
                    break
                fi
            done
            if [ $_tmpdetect != foundviascl ]; then
                #still missing, try via module load (this is more error prone)
                _tmpdetect=$( { module avail --terse --verbose 2>&1 | grep '^gcc/'|sort -nr|head -1  ; }  )
                if [ x"$_tmpdetect" != "x" ]; then
                    echo "  => Loading module $_tmpdetect for modern gcc."
                    module load "$_tmpdetect"
                fi
            fi
        fi
        _tmpdetect=$(which python3 >& /dev/null && python3 -c 'import sys; print("bad" if sys.version_info[0:2]<(3,6) else "ok")'||echo bad)
        if [ $_tmpdetect != ok ]; then
            echo "Centos/RHEL with missing or outdated python3 detected."
            #Missing python3 or python3 is too old, try to fix with SCL. Prefer known to hypothetical versions.
            for _tmpdetect in rh-python36 rh-python35 rh-python37 rh-python38 rh-python39; do
                if [ -f /etc/scl/prefixes/$_tmpdetect ]; then
                    echo "  => Enabling $_tmpdetect SCL for modern Python."
                    source scl_source enable $_tmpdetect
                    break
                fi
            done
        fi
    fi
    unset _tmpdetect||true
fi
