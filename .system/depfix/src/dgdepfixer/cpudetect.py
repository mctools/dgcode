
from dgdepfixer.std import *
from dgdepfixer import allmods as DG

def get_n_cores(prefix=''):
    if os.path.exists('/proc/cpuinfo'):
        #linux
        n=0
        for l in open('/proc/cpuinfo'):
            if l.startswith('processor') and l.split()[0:2]==['processor',':']:
                n+=1
        if not n:
            print(prefix+'Warning: Could not determine number of processors. Assuming just one present (override with -jN)')
            return 1
        return n
    else:
        #osx
        (ec,n)=DG.sysutils.system('sysctl -n hw.ncpu',catch_output=True)
        if ec:
            print(prefix+'Warning: Could not determine number of processors. Assuming just one present (override with -jN)')
            return 1
        return int(n.strip())

def get_load(ncores,prefix=''):
    (ec,p)=DG.sysutils.system('/bin/ps -eo pcpu',catch_output=True)
    if ec:
        print(prefix+'Warning: Could not determine current CPU load. Assuming 0%.')
        return 0.0
    p=0.01*sum([float(x.replace(b',',b'.')) for x in p.split()[1:]])
    if p>ncores*1.5:
        print(prefix+'Warning: Could not determine current CPU load ("ps -eo pcpu" output was suspicous). Assuming 0%.')
        return 0.0
    from os import environ as env
    if env.get('GITHUB_SERVER_URL',''): #Ignore CPU load for better performance when using GitHub Runners (CI)
      return 0.0
    return p

def auto_njobs(prefix = ''):
    nc=get_n_cores(prefix)
    freecores=nc-get_load(nc,prefix)
    return max(1,int(0.5+0.98*freecores))
