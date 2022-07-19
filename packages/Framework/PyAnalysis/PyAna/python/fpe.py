import Core.misc
import Core.FPE

def disableFPEDuringCall(obj,fctname):
    if not Core.misc.is_debug_build():
        return
    orig = getattr(obj,fctname)
    def safefct(*args,**kwargs):
        with Core.FPE.DisableFPEContextManager():
            return orig(*args,**kwargs)
    setattr(obj,fctname,safefct)

__standardFixesDone = [False]
def standardMPLFixes():
    global __standardFixesDone
    if __standardFixesDone[0]:
        return
    __standardFixesDone[0] = True
    if not Core.misc.is_debug_build():
        return

    import matplotlib.pyplot
    import matplotlib.figure
    import matplotlib.scale
    disableFPEDuringCall(matplotlib.pyplot,'tight_layout')
    disableFPEDuringCall(matplotlib.figure.Figure,'tight_layout')
    disableFPEDuringCall(matplotlib.figure.Figure,'savefig')
    disableFPEDuringCall(matplotlib.pyplot,'show')
    disableFPEDuringCall(matplotlib.pyplot,'savefig')
    if hasattr(matplotlib.scale,'LogTransformBase'):
        disableFPEDuringCall(matplotlib.scale.LogTransformBase,'transform_non_affine')
