#!/usr/bin/env python
import argparse, os

from subprocess import call

from PSO.PSOManager import PSOManager

def KILL(log):
    raise RuntimeError('@@@ FATAL -- '+log)

if __name__ == '__main__':
    ### args
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config', dest='config',
                        action='store', default='',
                        help='path to PSO configuration file')
    parser.add_argument('-o', '--output-dir', dest='output_dir',
                        action='store', default='',
                        help='path to output directory')
    parser.add_argument('--skip-trees', dest='skip_trees',
                        action='store_true', default=False,
                        help='skip creation of signal/background trees')
    parser.add_argument('-v', '--verbose', dest='verbose',
                        action='store_true', default=False,
                        help='print additional debug information')
    parser.add_argument('--dryrun', dest='dryrun',
                        action='store_true', default=False,
                        help='enable dry-run mode (PSO set up but not executed)')
    opts, opts_unknown = parser.parse_known_args()
    ###

    log_prx = 'runPSO.py -- '

    if not opts.config    : KILL(log_prx+'unspecified path to PSO configuration file [-c]')
    if not opts.output_dir: KILL(log_prx+'unspecified path to output directory [-o]')

    if not os.path.isfile(opts.config): KILL(log_prx+'PSO configuration file not found [-c]: '+opts.config)

    if os.path.exists(opts.output_dir): KILL(log_prx+'path to output directory already exists [-o]: '+opts.output_dir)
    ###

    DATA_SUBDIR = 'InitData'

    print('Created dir '+opts.output_dir)
    call(['mkdir', '-p', opts.output_dir])
    call(['mkdir', '-p', opts.output_dir+'/'+DATA_SUBDIR])

    call(['cp', opts.config, opts.output_dir+'/'+DATA_SUBDIR+'/config.txt'])

    if not opts.skip_trees:
       call(['root', '-b', '-q', 'PSO/PrepareTrees.C+(\"'+opts.config+'\", \"'+opts.output_dir+'/'+DATA_SUBDIR+'\")'])

    PSO = PSOManager(opts.output_dir, DATA_SUBDIR, opts.verbose, opts.config)
    PSO.CompileAndSetupClientExecutable()
    PSO.InitParticles()

    if not opts.dryrun:
       PSO.RunPSO()
       PSO.PrintResult()
