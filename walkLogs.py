import sys
import os

#variableList=[]

#for inf in infiles:
  #intext=open(inf,"r")
  #inlines=list(intext)
  #for line in inlines:
    #if "UnusedVar" in line or "UsedVar" in line:
      #splitline=line.replace("\n","").split()
      #thisvarname=splitline[1]
      #thismode=splitline[0]
      #foundvar=False
      #thisIVar=0
      #for ivar,var in enumerate(variableList):
        #if var[0]==thisvarname:
          #thisIVar=ivar
          #foundvar=True
          #break

      #if foundvar==False:
        #variableList.append([thisvarname,0,0])
        #thisIVar=-1
      #if thismode=="UnusedVars":
        #variableList[thisIVar][2]+=1
      #elif thismode=="UsedVar":
        #variableList[thisIVar][1]+=1
  #intext.close()

def SumVariableLists(listOflists):
  totalList=listOflists[0]
  for plist in listOflists[1:]:
    for var in plist:
      for totalvar in totalList:
	if var[0]==totalvar[0]:
	  totalvar[1]+=var[1]
	  totalvar[2]+=var[2]
	  break
  return totalList


def printVariableList(varlist):
  for var in varlist:
    print '%-60s%-5i%-5i' % (var[0], var[1], var[2])

class Particle:
  def __init__(self, path):
    self.path=path
    self.FinalVariableList=[]
    self.LogFiles=[]
    self.JobIDs=[]
    self.LogFileNameHead=""
    self.nIterations=0
    
    pfiles=os.listdir(self.path+"/logs/")
    for thisfile in pfiles:
      if ".o" in thisfile:
	splitfilename=thisfile.split(".o")
	#print splitfilename
	self.LogFileNameHead=splitfilename[0]+".o"
	self.JobIDs.append(splitfilename[1])
    self.JobIDs.sort()
    for job in self.JobIDs:
      self.LogFiles.append(self.path+"/logs/"+self.LogFileNameHead+job)
      self.nIterations+=1
    #print self.LogFiles

  def GetNIterations(self):
    return self.nIterations

  
  def ReadFinalVariables(self):
    for it, job in enumerate(self.LogFiles):
      intext=open(job,"r")
      inlines=list(intext)
      thisvarlist=[]
      for line in inlines:
        if "UnusedVar" in line or "UsedVar" in line:
	  splitline=line.replace("\n","").split()
	  thisvarname=splitline[1]
	  thismode=splitline[0]
	  thisvarlist.append([thisvarname,0,0])
	  if thismode=="UnusedVars":
            thisvarlist[-1][2]+=1
          elif thismode=="UsedVar":
            thisvarlist[-1][1]+=1
      intext.close()
      self.FinalVariableList.append(thisvarlist)
	
  def GetFinalVariablesOfIteration(self,it):
    #print self.path, it, self.FinalVariableList[it]
    return self.FinalVariableList[it]
  
  def GetTotalFinalVariables(self):
    listOflists=[]
    for it in range(self.nIterations):
      listOflists.append(self.GetFinalVariablesOfIteration(it))
    print "after all iterations ", self.path
    totallist=SumVariableLists(listOflists)
    print totallist
    return totallist
      
class VariableAnalyzer:
  def __init__(self):
    self.Particles=[]
    self.nIterations=0
    particlePathList=os.listdir("Particles")

    for particle in particlePathList:
      if "Particle" in particle:
        self.Particles.append(Particle("Particles/"+particle))
        self.nIterations = self.Particles[-1].GetNIterations()
        self.Particles[-1].ReadFinalVariables()
      #self.Particles[-1].GetFinalVariablesOfIteration(0)
      #self.Particles[-1].GetTotalFinalVariables()
      #print self.nIterations

  def GetFinalVariablesOfIteration(self,it):
      listOflists=[]
      for p in self.Particles:
        listOflists.append(p.GetFinalVariablesOfIteration(it))
      itlist=SumVariableLists(listOflists)
      print "\nIteration ", it, ":"
      printVariableList(itlist)
			   
    


myAna=VariableAnalyzer()
for it in range(myAna.nIterations):
  myAna.GetFinalVariablesOfIteration(it)
