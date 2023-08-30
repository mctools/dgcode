
"""Internal module providing ctypes-based hooks into the compiled NCrystal shared library"""

################################################################################
##                                                                            ##
##  This file is part of NCrystal (see https://mctools.github.io/ncrystal/)   ##
##                                                                            ##
##  Copyright 2015-2023 NCrystal developers                                   ##
##                                                                            ##
##  Licensed under the Apache License, Version 2.0 (the "License");           ##
##  you may not use this file except in compliance with the License.          ##
##  You may obtain a copy of the License at                                   ##
##                                                                            ##
##      http://www.apache.org/licenses/LICENSE-2.0                            ##
##                                                                            ##
##  Unless required by applicable law or agreed to in writing, software       ##
##  distributed under the License is distributed on an "AS IS" BASIS,         ##
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  ##
##  See the License for the specific language governing permissions and       ##
##  limitations under the License.                                            ##
##                                                                            ##
################################################################################

__all__ = ['_np',
           '_ensure_numpy',
           '_np_linspace',
           '_np_geomspace',
           '_np_logspace' ]

try:
    import numpy as _np
except ImportError:
    _np = None

def _ensure_numpy():
    if not _np:
        from .exceptions import NCException
        raise NCException("Numpy not available - array based functionality is unavailable")
    return _np

def _np_linspace(start,stop,num=50):
    """linspace with reproducible endpoint value"""
    _ensure_numpy()
    assert num >= 2
    l = _np.linspace(start,stop,num)
    l[0] = start
    l[-1] = stop
    return l

def _np_geomspace(start,stop,num=50):
    """geomspace with reproducible endpoint value"""
    _ensure_numpy()
    assert num >= 2
    l = _np.geomspace(start,stop,num)
    l[0] = start
    l[-1] = stop
    return l

def _np_logspace(start,stop,num=50):
    """logspace with reproducible endpoint value"""
    _ensure_numpy()
    assert num >= 2
    l = _np.logspace(start,stop,num)
    l[0] = 10.0**start
    l[-1] = 10.0**stop
    return l
