__metaclass__ = type#py2 backwards compatibility
def run(cmd):
    if not isinstance(cmd,list):
        cmd=cmd.split()
    import subprocess
    output=None
    try:
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output = p.communicate()[0]
    except:
        return (1,'')
        pass
    return (p.returncode,output)


import sys
class _mod_call:
  def __call__(self,cmd):
    return run(cmd)
sys.modules[__name__] = _mod_call()
