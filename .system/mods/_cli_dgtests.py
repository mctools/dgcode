#!/usr/bin/env python3
import os
import sys
import fnmatch
from optparse import OptionParser#FIXME: deprecated, use argparse instead!
join=os.path.join
import shutil
import shlex
import pathlib

from .conf import runnable_is_test
from .utils import mkdir_p
from .utils import system

import dgbuild.cfg
from . import testxml

def perform_tests(testdir,installdir,njobs,prefix,nexcerpts,filters,do_pycoverage):

    col_ok = '\033[92m'
    col_bad = '\033[91m'
    col_end = '\033[0m'

    print (prefix+'Running tests in %s:\n'%testdir+prefix)
    sys.stdout.flush()
    sys.stderr.flush()

    assert not os.path.exists(testdir)
    testdir=os.path.abspath(testdir)
    line_hr = "%s ---------------------------------------+-----------+--------+----------+------------------"%prefix
    line_titles = "%s  Test job results                      | Time [ms] | Job EC | Log-diff | Trouble info"%prefix
    header='\n'.join([line_hr,line_titles,line_hr])
    footer=line_hr

    logdir=join(installdir,'tests/testref')
    #bindirs=[join(installdir,'bin'),join(installdir,'scripts')]
    logfiles = set(fn for fn in os.listdir(logdir) if fn.endswith('.log')) if os.path.exists(logdir) else set()

    def ldtest(d):
        if os.path.exists(d):
            for f in os.listdir(d):
                if os.path.isdir(os.path.join(d,f)):
                    continue#skip some weird .DSYM dirs for fortran apps on osx
                if f+'.log' in logfiles or runnable_is_test(f):
                    yield join(d,f)
    tests=set()
    testinfo = {}
    for p in (p for _,p in dgbuild.cfg.pkgs.items() if p['enabled']):
        reflogs, runnables = p['reflogs'],p['runnables']
        for runnable in runnables:
            hasreflog = '%s.log'%runnable in reflogs
            if hasreflog or runnable_is_test(runnable):
                if filters:
                    selected=False
                    for fltr in filters:
                        fltr=fltr.strip()
                        invert=False
                        if fltr.startswith('!'):
                            fltr=fltr[1:].strip()
                            invert=True
                        if not fltr:
                            continue
                        if not fnmatch.fnmatch(runnable,fltr) == invert:
                            selected=True
                    if not selected:
                        continue
                assert runnable not in tests
                tests.add(runnable)
                testinfo[runnable] = {'pkgname':p['name']}

    if not tests:
        print (header)
        print ('%s  n/a'%prefix)
        print (footer)
        return 0

    mkdir_p(testdir)
    mf=open(join(testdir,'Makefile'),'w')

    mf.write('TESTDIR:=%s\n\n'%os.path.abspath(testdir))

    mf.write('.DEFAULT: all\n\n')
    mf.write('.PHONY: all\n\n')

    alltests=[]

    if do_pycoverage:
        coverage_out_dir = pathlib.Path(testdir) / 'pycoverage'
        coverage_out_dir.mkdir(parents=True, exist_ok=False)
        coverage_all_out_files = []

    for t in tests:
        bn=os.path.basename(t)
        td=join(testdir,bn)
        mkdir_p(td)
        tf=open(join(td,'run.sh'),'w')
        tf.write('#!/usr/bin/env bash\n')
        cmdstr = bn
        if do_pycoverage:
            _whichbn = shutil.which(bn)
            _ = pathlib.Path( _whichbn ) if _whichbn else None
            if _ and _.exists() and _.read_bytes().startswith(b'#!/usr/bin/env python3'):
                coverage_outfile = coverage_out_dir / ('%s.coverage'%bn)
                coverage_all_out_files.append( coverage_outfile )
                cmdstr = 'python3 -m coverage run --data-file=%s %s'%(coverage_outfile,_whichbn)
        tf.write('mkdir rundir && cd rundir && touch ../time_start && %s &> ../output.log \n'%cmdstr)
        tf.write('EC=$?\n')
        tf.write('touch ../time_end\n')
        tf.write('echo $EC > ../ec.txt\n')
        tf.write('python3 -c \'import os.path;print (1000.0*(os.path.getmtime("../time_end")-os.path.getmtime("../time_start")))\' > ../timing_ms\n')
        if bn+'.log' in logfiles:
            ref=join(installdir,'tests/testref/%s.log'%bn)
            tf.write("""if [ $EC == 0 ]; then
      diff -a --ignore-all-space %s ../output.log > ../refdiff.log || diff -a --ignore-all-space -y %s ../output.log > ../refdiff_sidebyside.log; EC=$?; echo $EC > ../ecdiff.txt
    fi"""%(ref,ref))
        tf.close()

        alltests+=[bn]
        mf.write('.PHONY: %s\n\n'%bn)
        mf.write('%s:\n'%bn)
        mf.write('\t@cd ${TESTDIR}/%s && chmod +x run.sh && ./run.sh'%bn)
        mf.write('\n\n')

    mf.write('all: %s\n\n'%(' '.join(alltests)))
    mf.close()
    os.environ['DISPLAY']=''
    os.environ['PYTHONUNBUFFERED']='1'
    ec_global=system('make -j%i -k -f %s/Makefile all'%(njobs,testdir))
    rep=[]
    for t in alltests:
        td=join(testdir,t)
        ecdiff=None if not os.path.exists(join(td,'ecdiff.txt')) else int(open(join(td,'ecdiff.txt')).read())
        rep+=[(t,int(open(join(td,'ec.txt')).read()),ecdiff)]
    rep.sort()
    excerpts_to_print=[]
    print (header)
    for t,ec,ecdiff in rep:
        time_ms=float(open(join(testdir,t,'timing_ms')).read())
        ecstr='FAIL' if ec else ' OK '
        logdiffstr=' -- '
        if ecdiff is not None:
            logdiffstr='FAIL' if ecdiff else ' OK '
        info='--'
        if ec!=0 or ( (ecdiff is not None) and ecdiff):
            info=os.path.realpath(join(testdir,t))
            ec_global=1
            if nexcerpts>0:
                excerpts_to_print += [(t,testdir,'output.log' if ec!=0 else 'refdiff.log')]
        print ("%s  %-37s |  %6.0f   |  %s  |   %s   | %s"%(prefix,t,time_ms,
                                                           ecstr,
                                                           logdiffstr,info))
        logdiffok = (ecdiff is None or not ecdiff)
        testinfo[t].update( dict(time_ms = time_ms,
                                 exitcode = ec,
                                 logfile = join(testdir,t,'output.log'),
                                 logdiffok = logdiffok,
                                 logdifffile = (join(testdir,t,'refdiff.log') if not logdiffok else None)))
    if do_pycoverage:
         _cov_infiles = ' '.join(shlex.quote(str(e.absolute().resolve())) for e in coverage_all_out_files)
         _cov_outfile = coverage_out_dir / 'combined.coverage'
         ec = system(f'cd {shlex.quote(str(coverage_out_dir))} && '
                     f'python3 -mcoverage combine --keep --data-file={shlex.quote(str(_cov_outfile.name))} {_cov_infiles}')
         if ec!=0:
             print(prefix,"ERROR: py-coverage combine step failed!")
         else:
             reportdir_quoted = shlex.quote(str((coverage_out_dir/"htmlreport").absolute().resolve()))
             ec = system('python3 -mcoverage html '
                         f'--data-file={shlex.quote(str(_cov_outfile.absolute().resolve()))}'
                         f'  --directory={reportdir_quoted} --ignore-errors')
             if ec!=0:
                 print(prefix,"ERROR: py-coverage html report generation failed!")
             else:
                 print(prefix,f"Pycoverage report available in: {reportdir_quoted}/index.html")

    assert len(alltests)==len(set(alltests))

    print (footer)

    do_junitxml = True
    if do_junitxml:
        testxmlwriter = testxml.TestXMLWriter()
        for testcmd in tests:
            nfo = testinfo[testcmd]
            nfo['time_ms'] = 100.0
            test = testxml.Test(testcmd,nfo['pkgname'],nfo['time_ms']*0.001)
            if nfo['exitcode'] != 0:
                test.set_run_failure(nfo['logfile'])
            elif not nfo['logdiffok']:
                test.set_reflog_failure(nfo['logdifffile'])
            testxmlwriter.add_test(test)
        outfile = pathlib.Path(testdir) / 'dgtest_results_junitformat.xml'
        with outfile.open('wt') as f:
            for line in testxmlwriter.generate_xml():
                f.write('%s\n'%line)
        print('%s\n%s  Test results are also summarised in %s\n%s'%(prefix,prefix,outfile.name,prefix))

    if excerpts_to_print:
        for t,testdir,logname in excerpts_to_print:
            print ('\n====>\n====> First %i lines from %s/%s:\n====>'%(nexcerpts,t,logname))
            system('head -%i %s'%(nexcerpts,os.path.join(testdir,t,logname)))
            print ('====> (end of %s/%s)\n'%(t,logname))
            print ('\n====>\n====> Last %i lines from %s/%s:\n====>'%(nexcerpts,t,logname))
            system('tail -%i %s'%(nexcerpts,os.path.join(testdir,t,logname)))
            print ('====> (end of %s/%s)\n'%(t,logname))

    sys.stdout.flush()
    sys.stderr.flush()

    if ec_global:
        print (prefix+'  %sERROR: Some tests failed!%s'%(col_bad,col_end))
        print (prefix)
    else:
        print (prefix+'  %sAll tests completed without failures!%s'%(col_ok,col_end))
        print (prefix)
    return 0 if ec_global==0 else 1

def parse_cmdline_args():
    parser = OptionParser(usage='%prog [options]')
    parser.add_option("-j", "--jobs",
                      type="int", dest="njobs", default=1,
                      help="use up to N parallel processes",metavar="N")
    parser.add_option("-p", "--prefix",type='string',help="prefix output",
                      action='store', dest="prefix", default='')
    parser.add_option("-d", "--dir",type='string',help="directory for running tests",
                      action='store', dest="dir", default='')
    parser.add_option("-e", "--excerpts",
                      type="int", dest="nexcerpts", default=0,
                      help="Show N first and last lines of each log file in failed tests",metavar="N")
    parser.add_option("-f", "--filter",
                      type="str", dest="filters", default='',
                      help=("Only run tests with names matching at least one of provided patterns (use wildcards"+
                            " and comma separation to specify, and prefix patterns with '!' to negate)"),metavar="PATTERN")
    parser.add_option("--pycoverage",action="store_true",dest="pycoverage", default=False, help="Run python scripts under the python coverage module.")

    (opt, args) = parser.parse_args()
    if args:
        parser.error('Unrecognised arguments: %s'%(' '.join(args)))
    if not opt.dir:
        fallback=os.path.join(os.getcwd(),'tests')
        if os.path.exists(fallback):
            parser.error('You must supply a desired test directory or be in a directory without a tests/ subdirectory')
        opt.dir=fallback
    if os.path.exists(opt.dir):
        parser.error('Test directory must not exist already')

    instdir=os.getenv("ESS_INSTALL_PREFIX")
    assert instdir and os.path.isdir(instdir)
    opt.instdir = instdir
    return opt

def main():
    opt = parse_cmdline_args
    ec = perform_tests(opt.dir,instdir,njobs=opt.njobs,prefix=opt.prefix,nexcerpts=opt.nexcerpts,
                       do_pycoverage = opt.pycoverage,
                       filters=[fltr.strip() for fltr in opt.filters.split(',') if fltr.strip()])
    sys.exit(ec)

if __name__=='__main__':
    main()