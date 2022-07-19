"""Collect all standard imports here and use from dgdepfixer.std import * elsewhere. Exception: Those that might be missing e.g. venv."""

import argparse
import builtins
import collections
import contextlib
import datetime
import errno
import fnmatch
import glob
import importlib
import json
import os
import pathlib
import pickle
import pkgutil
import platform
import shlex
import shutil
import ssl
import stat
import subprocess
import sys
import tarfile
import tempfile
import time
import traceback
import urllib
import urllib.error
import urllib.request

#Additional "standard" functions we want to provide/override:
def print(*args, **kwargs):
    #Version which flushes output by default
    if 'flush' in kwargs:
        return builtins.print(*args, **kwargs)
    else:
        return builtins.print(*args, flush = True, **kwargs)

def errprint(*args, **kwargs):
    #Version which use stderr + flushes output by default
    return print(*args, file=sys.stderr, **kwargs)
