import sys
if not ( (3,9) <= sys.version_info[0:2] ):
    from . import error
    error.error('Python not found in required version (3.9+ required).')
