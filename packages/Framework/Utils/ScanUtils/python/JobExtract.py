from __future__ import print_function
def flatten_pars(setup):
    """Merge all geo/gen/filter/userdata/metadata/g4cmd info from GriffDataRead::Setup into a single dict (assume no clashes)"""
    if hasattr(setup,'_flatpars'):
        return setup._flatpars
    pars={}
    pars.update(setup.geo().asDict())
    pars['_geoname_']=setup.geo().getName()
    pars.update(setup.gen().asDict())
    pars['_genname_']=setup.gen().getName()
    if setup.hasFilter():
        pars.update(setup.getFilter().asDict())
        pars['_filtername_']=setup.getFilter().getName()
    pars.update(setup.metaData())
    pars.update(setup.userData())
    pars['g4cmds'] = str(setup.cmds())#flatten like this?
    setup._flatpars = pars
    return pars

def interesting_pars(setups,always=set(),never=set(),do_print=True):
    """first parameter is a list of ScanLoader.ScanJob's or GriffDataRead.Setup's"""
    assert setups
    from GriffDataRead import Setup
    if not isinstance(setups[0],Setup) and hasattr(setups[0],'setup'):
        #is probably a list of ScanJob instances rather than Setup instances:
        setups=[j.setup() for j in setups]
    flat_setups = [set(flatten_pars(setup).items()) for setup in setups]
    boring_pars = flat_setups[0].intersection(*(flat_setups[1:]))
    boring_pars_names = set(k for k,v in boring_pars)
    boring_pars_names -= always
    boring_pars_names.update(never)
    all_pars = []
    for fs in flat_setups:
        all_pars += list((par,val) for par,val in fs if not par in boring_pars_names)
    all_pars.sort()
    import itertools
    if do_print:
        print("Interesting parameters are:")
        for par, vals in itertools.groupby(all_pars, lambda x: x[0]):
            print("      %s:"%par,' '.join(['%s [%i]'%(key[1],len(list(group))) for key, group in itertools.groupby(vals)]))
    return [par for par, vals in itertools.groupby(all_pars, lambda x: x[0])]

def summarise_geogenpars(scanjobs,ignore_same=False):
    #todo: filter parameters + metadata as well!
    import Utils.ParametersUtils
    geos=[j.setup().geo() for j in scanjobs]
    gens=[j.setup().gen() for j in scanjobs]
    print("============ Geometry parameters =============")
    (geosame,geodiff)=Utils.ParametersUtils.summarise_params(geos,ignore_same)
    print("============ Generator parameters ============")
    (gensame,gendiff)=Utils.ParametersUtils.summarise_params(gens,ignore_same)
    print("==============================================")
    return (geosame,geodiff,gensame,gendiff)

def geo_parvals(parname):
    vals=set()
    for j in scanjobs:
        eval('vals.add(j.setup().geo().%s)'%parname)
    l=list(vals)
    l.sort()
    return l

def select(selector):
    out=[]
    for j in scanjobs:
        if selector(j.setup()):
            out+=[j]
    return out

def scanjobs_groups(pars_varying_within_group,jobfilter=None):
    groups2jobs={}
    for j in scanjobs:
        if jobfilter and not jobfilter(j):
            continue
        groupkey=tuple(sorted((k,v) for k,v in j.opts.items() if not k in pars_varying_within_group))
        if groupkey in groups2jobs: groups2jobs[groupkey] += [j]
        else: groups2jobs[groupkey] = [j]
    if jobfilter and not groups2jobs:
        import inspect
        print("ERROR: No jobs selected by filter!!")
        print(inspect.getsource(jobfilter))
        assert False
    return groups2jobs

def group(jobs,groupfunc,selectfunc=None):
    d={}
    for j in jobs:
        if selectfunc==None or selectfunc(j):
            d.setdefault(groupfunc(j),[]).append(j)
    return d

def merge_hists_for_similar_jobs(joblist,quiet=False):
    n_orig=len(joblist)
    n_merge=0
    out=[]
    while joblist:
        j=joblist.pop()
        simjobs = [jj for jj in joblist if flatten_pars(jj.setup())==flatten_pars(j.setup())]
        for jj in simjobs:
            joblist.remove(jj)
            j.histcol().merge(jj.histcol())
            n_merge += 1
        out+=[j]
    joblist+=out
    if not quiet:
        print('merge_hists_for_similar_jobs: Reduced %i jobs to %i jobs via %i merges'%(n_orig,len(joblist),n_merge))
