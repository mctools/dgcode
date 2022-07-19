__author__ = "Thomas Kittelmann"
__copyright__ = "Copyright 2019"
__version__ = "0.1.0"#Important: Update this whenever any significant changes occur that might affect the installations created with --create
__maintainer__ = "Thomas Kittelmann"
__email__ = 'ue.sse@nnamlettik.samoht'[::-1]

swversions={
    #recommended versions for all deps are typically set to the most recent production release at the time of testing.
    'CMake': dict(oldest='3.12.3',recommended='3.20.5',newest=None),#v3.8.2 supports C++17, v3.12.4 supports FindPython3 and C++20. v3.12.3 comes with Conda, we hope it is good enough. v3.15 has improved search policies for python.
    'git':dict(oldest='1.8.3.1',recommended='2.17.1',newest=None),#v1.8.3.1 is available out of the box on lxplus and DMSC.
    'pymod:numpy':dict(oldest='1.17.0',recommended='1.22.2'),#1.17.0 is first release without python2 support.
    'pymod:matplotlib':dict(oldest='3.0.2',recommended='3.5.1'),#3.0.0 is first release without python2 support. NB: DMSC with 3.5 only supports up to 3.0.3 (THIS COMMENT IS OUTDATED DMSC NOW HAVE 3.6). NB: Matplotlib 3.2.1 introduced dependency on cm-super font, which dgdepfixer would need to look for!!! See also DGSW-520 before updating recommended version.
    'pymod:scipy':dict(oldest='1.3.0',recommended='1.6.3'),#1.3.0 is first release without python2 support
    'pymod:pandas':dict(oldest='0.24.2',recommended='1.2.4'),
    'pymod:mcpl':dict(oldest='1.3.0',recommended='1.3.2'),#1.3.0 has some python api issues, but we don't want to make trouble for users right now.
    'pymod:ptpython':dict(oldest='2.0.4',recommended='3.0.20'),
    #TODO: Other interesting ones: 'h5py','pyqt5', ipython,... (note: 'PyNE' is NOT the nuclear one)
}

import sys
if sys.version_info[0:2]<(3,7):
    #NB: 1.20.3 needed to resolve DGSW-520, but numpy 1.20.x requires python 3.7 or later!!!
    swversions['pymod:numpy']['recommended']='1.19.5'
    #Possibly the same issue for other components (saw latest version missing on python 3.6):
    swversions['pymod:pandas']['recommended']='1.1.5'
    swversions['pymod:scipy']['recommended']='1.5.4'
    #Latest versions of matplotlib compatible with Python 3.6
    swversions['pymod:matplotlib']['recommended']='3.3.4'  

