from . import utils

def cluster_ncpus():
    #if running at cluster via slurm, respect SLURM_CPUS_PER_TASK limit
    from os import environ as env
    cname,cncpu = env.get('SLURM_CLUSTER_NAME','').strip(),env.get('SLURM_CPUS_PER_TASK','notfound').strip()
    if cname.lower()=='dmsc' and cncpu.isdigit():
        return int(cncpu)

def get_n_cores(prefix):
    import os
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
        (ec,n)=utils.run('sysctl -n hw.ncpu')
        if ec:
            print(prefix+'Warning: Could not determine number of processors. Assuming just one present (override with -jN)')
            return 1
        return int(n.strip())

def get_load(ncores,prefix):
    (ec,p)=utils.run('/bin/ps -eo pcpu')
    if ec:
        print(prefix+'Warning: Could not determine current CPU load. Assuming 0%.')
        return 0.0
    if hasattr(p,'decode'):
        p=p.decode('ascii')
    p=0.01*sum([float(x.replace(',','.')) for x in p.split()[1:]])
    if p>ncores*1.5:
        print(prefix+'Warning: Could not determine current CPU load ("ps -eo pcpu" output was suspicous). Assuming 0%.')
        return 0.0
    from os import environ as env
    if env.get('GITHUB_SERVER_URL',''): #Ignore CPU load for better performance when using GitHub Runners (CI)
      return 0.0
    return p

def auto_njobs(prefix):
    n_reserved = cluster_ncpus()
    if n_reserved:
        return n_reserved
    nc=get_n_cores(prefix)
    freecores=nc-get_load(nc,prefix)
    return max(1,int(0.5+0.98*freecores))
