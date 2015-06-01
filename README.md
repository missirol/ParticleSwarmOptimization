# ParticleSwarmOptimization

Implementation of the particle swarm algorithm to optimize the paramters and Input Variables of TMVA Classificators.  
The program will create a swarm of Classifiers in the paramter space.  
Each classifier will try to improve its input variable combination.  
Classifiers move in the paramter space using the information of the whole swarm.  
Utilizes batch system to parallelize classifier training.  


1) clone the repository  

2) you might want to rebuild the executable which is run on the batch system  
   cd PSO  
   g++ -o ../InitData/Particle Particle.C `root-config --cflags --glibs` -lTMVA  
   also do this if you change it  


2) change Example_PSOConfig to suit your needs  
     should work on ekp and NAF batch system, for other batch system change PSO/QueHelper.py  
     play around with the swarm parameters  
     choose the Figure of Merit to optimize  
     specify TMVA factory and method options  
     declare coordinate space you want to search  
     specify Signal and Background trees   
     Variables the swarm starts with   
     pool of additional Variables the swarm will try  
    
3) Start the Optimization with  
    python RunPSO.py Example_PSOConfig.txt  
    
4) The ten best classifiers are writen to PSOResult.txt  
   The best classifier and all necessary information is written to a file starting with "FinalMVAConfig_"  
   
   
