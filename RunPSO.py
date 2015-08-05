#!/usr/bin/env python
from PSO.PSOManager import PSOManager
import sys
from subprocess import call

if len(sys.argv)<2:
  print "usage:"
  print "python PSO.py [OPTION] FILE_WITH_PSO_CONFIG"
  print "[-t] skip tree creating step, for example if you already did it for a previous run" 
  print "[-v] print additional debug information" 
  exit(0)
  
currentPath = sys.path[0]+"/"

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
