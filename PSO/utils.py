#!/usr/bin/env python
import os

def KILL(log):
    raise SystemExit('\n '+'\033[1m'+'@@@ '+'\033[91m'+'FATAL'  +'\033[0m'+' -- '+log+'\n')
# --

def WARNING(log):
    print '\n '+'\033[1m'+'@@@ '+'\033[93m'+'WARNING'+'\033[0m'+' -- '+log+'\n'
# --

def EXE(cmd, suspend=True, verbose=False, dry_run=False):
    if verbose: print '\033[1m'+'>'+'\033[0m'+' '+cmd
    if dry_run: return

    _exitcode = os.system(cmd)

    if _exitcode and suspend: raise SystemExit(_exitcode)
# --
