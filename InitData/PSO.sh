export VO_CMS_SW_DIR=/cvmfs/cms.cern.ch
export SCRAM_ARCH=slc5_amd64_gcc462
source $VO_CMS_SW_DIR/cmsset_default.sh
cd /portal/ekpcms5/home/karimel/CMSSW_5_3_15_patch1/src
eval `scram runtime -sh`
cd /portal/ekpcms5/home/karimel/TMVA/TMVA/TMVASubBDT/scripts/PSO/Particle1
./Particle