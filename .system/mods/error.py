from __future__ import print_function

# Custom Exception class to distinguish exceptions thrown by our code from other exceptions.
#
# Intended usage:
#
# import error
# error.error('some error message')

class Error(Exception):
    pass
def error(*args):
    raise Error('\n'.join(args))

#Used to exit the programme after catching and dealing with an Error at a lower level.
class CleanExit(Exception):
    pass
def clean_exit(ec):
    ce=CleanExit()
    ce._the_ec=ec
    raise ce

def print_traceback(exc,prefix=''):
    print (prefix)
    import traceback
    print ("%s----begin traceback---"%prefix)
    if hasattr(exc,'__traceback__'):
        traceback.print_exception(type(exc),exc,exc.__traceback__)
    else:
        #This can only happen in python2:
        traceback.print_exc(exc)
    print ("%s----end traceback---"%prefix)
    print (prefix)
