#We use a little class/sys.modules trick to allow both: G4Launcher(..) and
#G4Launcher.Launcher(..)

class Launcher:
    def __init__(self):
        """<internal>"""
        import G4Launcher._launcher
        self.__Launcher = G4Launcher._launcher.Launcher
        self.__getTheLauncher = G4Launcher._launcher.getTheLauncher
        self.__g4version = G4Launcher._launcher.g4version()
    def Launcher(self,*args):
        """Create and return Launcher object"""
        return self.__Launcher(*args)
    def getTheLauncher(self):
        return self.__getTheLauncher()
    def __call__(self,*args):
        """Create and return Launcher object"""
        return self.__Launcher(*args)
    def g4version(self):
        """Returns g4 version as integer (for instance 1003 means version 10.0.p03)"""
        return self.__g4version
import sys
sys.modules[__name__] = Launcher()
