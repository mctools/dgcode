def parse_cmdline_args():
    from optparse import OptionParser#FIXME: deprecated, use argparse instead!
    import os
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

    from . import dirs
    instdir = dirs.installdir
    assert instdir and os.path.isdir(instdir)
    opt.instdir = instdir
    return opt

def main():
    import sys
    opt = parse_cmdline_args()
    from .testlauncher import perform_tests
    ec = perform_tests( opt.dir,
                        opt.instdir,
                        njobs=opt.njobs,
                        prefix=opt.prefix,
                        nexcerpts=opt.nexcerpts,
                        do_pycoverage = opt.pycoverage,
                        filters=[fltr.strip() for fltr in opt.filters.split(',') if fltr.strip()] )
    sys.exit(ec)

if __name__=='__main__':
    main()
