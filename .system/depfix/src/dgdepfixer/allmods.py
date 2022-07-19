"""meta-module which imports all modules (except inst_XXX.py + std.py) so other
   modules can have simply start with:

     from dgdepfixer.std import *
     from dgdepfixer import allmods as DG

   Then they have access to our own modules as e.g. DG.sysutils and standard
   modules as os, sys, etc. as usual.
"""

from dgdepfixer import __version__,__email__,__path__,swversions

#Note: ordered roughly by dependencies, to avoid import errors:
import dgdepfixer.utils as utils
import dgdepfixer.sysutils as sysutils
import dgdepfixer.cpudetect as cpudetect
import dgdepfixer.fixosxlibinstallnames as fixosxlibinstallnames
import dgdepfixer.installutils as installutils
import dgdepfixer.platformutils as platformutils
import dgdepfixer.stdtool as stdtool
