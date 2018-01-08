#!/usr/bin/env python
import os, sys

from subprocess import call

from PSO.PSOManager import PSOManager

if len(sys.argv)<2:
  print "usage:"
  print "python RunPSO.py [OPTION] FILE_WITH_PSO_CONFIG"
  print "[-t] skip tree creating step, for example if you already did it for a previous run" 
  print "[-v] print additional debug information" 

  raise SystemExit

currentPath = sys.path[0]+"/"

if os.path.isdir(currentPath+'/Particles'):
   raise RuntimeError('"Particles" directory already exists')
else:
   print('Created dir '+currentPath+'/Particles')
   call(['mkdir', '-p', currentPath+'/Particles'])

MakeNewTrees=True
Verbose=False
PSOConfig=""
for arg in sys.argv[1:]:
  if "-t" in arg:
    MakeNewTrees=False
  if "-v" in arg:
    Verbose=True
  else:
    PSOConfig=arg

if MakeNewTrees==True:
  command="PSO/PrepareTrees.C+(\""+PSOConfig+"\")"
  call(["root","-b","-q",command])

PSO=PSOManager(currentPath,Verbose, PSOConfig)
PSO.CompileAndSetupClientExecutable()
PSO.SetupParticles()
PSO.InitParticles()
PSO.RunPSO()
PSO.PrintResult()
