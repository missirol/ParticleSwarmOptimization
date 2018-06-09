#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>

#include "TGraph.h"
#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TRandom3.h"

#include "TStopwatch.h"
#include "TMVA/IMethod.h"
#include "TMVA/MethodBase.h"
#include "TMVA/MethodBDT.h"
#include "TMVA/ResultsClassification.h"
#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TVirtualFitter.h"

TH1D* SignalForChi2;
TH1D* BackgroundForChi2;
bool Chi2ScaleBackground=true;
bool Chi2ScaleSignal=true;
bool Chi2DoSplusB=false;
Double_t TimeTotal=0.0;
Double_t TimeTreePreparation=0.0;
Double_t TimeTraining=0.0;
Double_t TimeTesting=0.0;
int NTrainings=3;




class BDTVar :public TObject {
  public:
    BDTVar();
    ~BDTVar();
    
    TString name;
//     Double_t FOMI;
//     Double_t KS;
//     Double_t Separation;
    
//     ClassDef(BDTVar,1);
    
};

BDTVar::BDTVar(){
 }

 BDTVar::~BDTVar(){
 }


double SimpleChi2(TH1D* histoData, TH1D* histoModel){
  double chi2=0.0;
  int nBins=histoData->GetNbinsX();
  for(int i=1;i<=nBins;i++){
    double d=histoData->GetBinContent(i);
    double m=histoModel->GetBinContent(i);
    double tmp = (d-m)*(d-m)/m;
    chi2+=tmp;
  }
  return chi2;
}

void myFCN(Int_t & nPar, Double_t * grad, Double_t &fval, Double_t *p, Int_t flag){
  Double_t scaleB=1.0;
  Double_t scaleS=1.0;
  
  if(Chi2ScaleBackground==true){scaleB=p[0];}
  if(Chi2ScaleSignal==true){scaleS=p[0];}
  
  TH1D* histoData=(TH1D*)BackgroundForChi2->Clone("histoData");
  if(Chi2DoSplusB==true){
    histoData->Add(SignalForChi2,1.0);
  }
  
  TH1D* histoModel=(TH1D*)BackgroundForChi2->Clone("histoData");
  histoModel->Scale(scaleB);
  histoModel->Add(SignalForChi2,scaleS);
  
  Double_t chi2=0.0;
  chi2=SimpleChi2(histoData,histoModel);
  fval=chi2;
}

Double_t GetChi2Value(Double_t bestMu){
  Double_t scaleB=1.0;
  Double_t scaleS=1.0;
  
  if(Chi2ScaleBackground==true){scaleB=bestMu;}
  if(Chi2ScaleSignal==true){scaleS=bestMu;}
  
  TH1D* histoData=(TH1D*)BackgroundForChi2->Clone("histoData");
  if(Chi2DoSplusB==true){
    histoData->Add(SignalForChi2,1.0);
  }
  
  TH1D* histoModel=(TH1D*)BackgroundForChi2->Clone("histoData");
  histoModel->Scale(scaleB);
  histoModel->Add(SignalForChi2,scaleS);
  
  Double_t chi2=0.0;
  chi2=SimpleChi2(histoData,histoModel);
  return chi2;
}

Double_t GetChi2FOM(TH1D* histoSignal,Double_t SignalWeight, TH1D* histoBackground,Double_t BackgroundWeight,TString mode="Chi2_B_muSB"){
  
  //prepare histograms
  Int_t nBinsForLimit=10;
  SignalForChi2=(TH1D*)histoSignal->Clone("SignalForChi2");
  BackgroundForChi2=(TH1D*)histoBackground->Clone("BackgroundForChi2");
  SignalForChi2->Rebin(int(10000/nBinsForLimit));
  BackgroundForChi2->Rebin(int(10000/nBinsForLimit));
  
  SignalForChi2->Scale(1.0/SignalForChi2->Integral());
  SignalForChi2->Scale(2.0*SignalWeight);
  BackgroundForChi2->Scale(1.0/BackgroundForChi2->Integral());
  BackgroundForChi2->Scale(2.0*BackgroundWeight);
  
  std::cout<<"Signal Integral "<<SignalForChi2->Integral()<<std::endl;
  std::cout<<"Background Integral "<<BackgroundForChi2->Integral()<<std::endl;

  SignalForChi2->SaveAs("HistoSignalForChi2.root");
  BackgroundForChi2->SaveAs("HistoBackgroundForChi2.root");

  if(mode=="Chi2_B_muSB"){
    Chi2DoSplusB=false;
    Chi2ScaleSignal=true;
    Chi2ScaleBackground=true;
  }
  else{std::cout<<"!!!!!!!!!!!!!!dont know chi2 mode!!!!!!!!!!!!!!!!!"<<std::endl;}
  
  //minimize
  TVirtualFitter::SetDefaultFitter("Minuit");
  TVirtualFitter * minuit = TVirtualFitter::Fitter(0,1);
  double arglist[10];
  double arglistVerbose[10];
  arglist[0]=5000;
  arglist[1]=0.01;
  arglistVerbose[0]=-1.0;
  minuit->ExecuteCommand("SET PRINT",arglistVerbose,2);
  minuit->SetParameter(0,"mu",6.0,0.5,0.0001,20.0);
  minuit->SetFCN(myFCN);
  minuit->ExecuteCommand("MIGRAD",arglist,2);
  Double_t bestMu = minuit->GetParameter(0);
  
  //get chi2 at minimal mu value
  
  if(mode=="Chi2_B_muSB"){
    Chi2DoSplusB=false;
    Chi2ScaleSignal=true;
    Chi2ScaleBackground=true;
  }
  
  Double_t fom = 0.0;
  if(mode=="Chi2_B_muSB"){
    fom = GetChi2Value(bestMu);
  }
  return fom;
}

 void DoTraining(std::vector<BDTVar*> UsedVars,std::vector<BDTVar*> UnUsedVars,TString FOMType, TString FactoryString, TString PrepString, TString SigWeight, TString BkgWeight, TString SignalTreeName, TString BackgroundTreeName, TString MethodType, TString MethodString, int particleNumber, Double_t* testFOM, Double_t* testKS,Int_t UseFixedTrainTestSplitting, TString PlotName="NONE" ){
   std::cout<<"----------------------------------------------------------------"<<std::endl;
   
   //prepare TMVA
   TMVA::IMethod* im;
   //timing studies
    TStopwatch* thisTimer = new TStopwatch();
    Double_t thisTreeTime=0.0;
    Double_t thisTrainingTime=0.0;
    Double_t thisTestingTime=0.0;
    
   TMVA::Tools::Instance();
   TString outfileName( "TMVAVars.root" );
   int nUsedVars=UsedVars.size();

   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );
   
   TMVA::Factory *factory = new TMVA::Factory( "TMVAVars", outputFile,FactoryString );
   std::cout<<MethodString<<std::endl;
   
   std::cout<<"Using Variables"<<std::endl;
   for(int k=0; k<UsedVars.size(); k++){
     std::cout<<k<<" "<<UsedVars.at(k)->name<<std::endl; 
     factory->AddVariable(UsedVars.at(k)->name);
   }

   thisTimer->Start();

   TFile *inputSTrain;
   TFile *inputBTrain;
   TTree *signalTrain;
   TTree *backgroundTrain;
   TFile *inputSTest;
   TFile *inputBTest;
   TTree *signalTest;
   TTree *backgroundTest;
   TFile *inputS;
   TFile *inputB;
   TTree *signal;
   TTree *background;

   Double_t wSfChi2=0.0;
   Double_t wBfChi2=0.0;

   if(UseFixedTrainTestSplitting == 1)
   {
     inputSTrain = TFile::Open("Signal_Train.root");
     inputBTrain = TFile::Open("Background_Train.root");

     signalTrain     = (TTree*) inputSTrain->Get(SignalTreeName);
     backgroundTrain = (TTree*) inputBTrain->Get(BackgroundTreeName);

     factory->AddSignalTree    (signalTrain,     1.0, TMVA::Types::kTraining);
     factory->AddBackgroundTree(backgroundTrain, 1.0, TMVA::Types::kTraining);

     inputSTest = TFile::Open("Signal_Test.root");
     inputBTest = TFile::Open("Background_Test.root");

     signalTest     = (TTree*) inputSTest->Get(SignalTreeName);
     backgroundTest = (TTree*) inputBTest->Get(BackgroundTreeName);

     factory->AddSignalTree    (signalTest,     1.0, TMVA::Types::kTesting);
     factory->AddBackgroundTree(backgroundTest, 1.0, TMVA::Types::kTesting);

     wSfChi2 = signalTest    ->GetMinimum(SigWeight) * signalTest    ->GetEntries();
     wBfChi2 = backgroundTest->GetMinimum(BkgWeight) * backgroundTest->GetEntries();
   }
   else
   {
     inputS = TFile::Open( "Signal.root" );
     inputB = TFile::Open( "Background.root" );

     signal     = (TTree*) inputS->Get(SignalTreeName);
     background = (TTree*) inputB->Get(BackgroundTreeName);

     wSfChi2 = (signal    ->GetMinimum(SigWeight) * signal    ->GetEntries());
     wBfChi2 = (background->GetMinimum(BkgWeight) * background->GetEntries());

     factory->AddSignalTree    (signal,     1.0);
     factory->AddBackgroundTree(background, 1.0);
   }
   factory->SetBackgroundWeightExpression( SigWeight );
   factory->SetSignalWeightExpression( BkgWeight );

   std::cout << "Running TMVA::Factory::PrepareTrainingAndTestTree(\"\", \"\", \""+PrepString+"\")" << std::endl;
   factory->PrepareTrainingAndTestTree("", "", PrepString);

   //check method and book it
   if(MethodType=="TMVA::Types::kBDT"){
     factory->BookMethod( TMVA::Types::kBDT,"myMVA", MethodString );
   }
   else if(MethodType=="TMVA::Types::kLikelihood"){
     factory->BookMethod( TMVA::Types::kLikelihood,"myMVA", MethodString );
   }
   else if(MethodType=="TMVA::Types::kMLP"){
     factory->BookMethod( TMVA::Types::kMLP,"myMVA", MethodString );
   }
   else{
     std::cout<<"dont know "<<MethodType<<std::endl;
     exit(1);
   }
   
   //timing studies
   thisTreeTime=thisTimer->RealTime();
   thisTimer->Start();

   std::cout << "Running TMVA::Factory::TrainAllMethods()" << std::endl;
   factory->TrainAllMethods();

   std::cout<<"--------Training Done------"<<std::endl;

   //timing studies
   thisTrainingTime=thisTimer->RealTime();
   thisTimer->Start();

   // ---- Evaluate all MVAs using the set of test events
   factory->TestAllMethods();
   im = factory -> GetMethod("myMVA");
   TMVA::MethodBase* myMethod = dynamic_cast<TMVA::MethodBase*>(im);
   Double_t err;
   myMethod->TestClassification();

   Double_t ROCC1 = myMethod->GetROCIntegral();
   Double_t ROC = myMethod->GetEfficiency("",TMVA::Types::kTesting, err);

   //buff is needed for KS test
   Double_t buff = myMethod->GetTrainingEfficiency("Efficiency:0.5");

   std::cout << "doing sep" << std::endl;

   Double_t sep = myMethod->GetSeparation();
//   Double_t sep=0.0;

   TMVA::DataSet* myData = myMethod->Data();
   myData->SetCurrentType(TMVA::Types::kTesting);
   TMVA::ResultsClassification* mvaRes = dynamic_cast<TMVA::ResultsClassification*>
       (myData->GetResults(myMethod->GetMethodName(),TMVA::Types::kTesting, TMVA::Types::kClassification) );
   TH1* mva_s = (TH1*) mvaRes->GetHist("MVA_S");
   TH1* mva_b = (TH1*) mvaRes->GetHist("MVA_B");
   TH1* mva_effS = (TH1*) mvaRes->GetHist("MVA_EFF_S");
   TH1* mva_effB = (TH1*) mvaRes->GetHist("MVA_EFF_B");
   
   TH1D* mva_s_high = (TH1D*) mvaRes->GetHist("MVA_HIGHBIN_S");
   TH1D* mva_b_high = (TH1D*) mvaRes->GetHist("MVA_HIGHBIN_B");
/*   TH1* mva_FOMCurve = (TH1*) mvaRes->GetHist("MVA_myMVA_rejBvsS");*/
   
   //do chi2 stuff
   Double_t chi2FOM=0.0;
   if(FOMType=="Chi2_B_muSB"){
     chi2FOM=GetChi2FOM(mva_s_high,wSfChi2,mva_b_high,wBfChi2,"Chi2_B_muSB");
   }
   
   
   int roccNBins=mva_effS->GetNbinsX();
   TGraph* mva_ROCCurve_H = new TGraph(roccNBins);
   for(int ib=1;ib<=roccNBins;ib++){
     Double_t eS=mva_effS->GetBinContent(ib);
     Double_t rB=1.0-mva_effB->GetBinContent(ib);
     mva_ROCCurve_H->SetPoint(ib,eS,rB);
   }
   mva_ROCCurve_H->Sort();
   if(PlotName!="NONE"){
     mva_ROCCurve_H->SaveAs(PlotName+".root");
   }

   int bin=0;
   bin=mva_effS->FindLastBinAbove(0.1)+1;
   Double_t effS_01 = mva_effS->GetBinContent(bin);
   Double_t rejB_01 = 1.0-mva_effB->GetBinContent(bin);
//    std::cout<<bin<<std::endl;
   std::cout<<"rejB at 0.1 effS "<<rejB_01<<" at "<<effS_01<<std::endl;
   bin=mva_effS->FindLastBinAbove(0.3)+1;
//    std::cout<<bin<<std::endl;
   Double_t effS_03 = mva_effS->GetBinContent(bin);
   Double_t rejB_03 = 1.0-mva_effB->GetBinContent(bin);
   std::cout<<"rejB at 0.3 effS "<<rejB_03<<" at "<<effS_03<<std::endl;
   bin=mva_effS->FindLastBinAbove(0.5)+1;
//    std::cout<<bin<<std::endl;
   Double_t effS_05 = mva_effS->GetBinContent(bin);
   Double_t rejB_05 = 1.0-mva_effB->GetBinContent(bin);
   std::cout<<"rejB at 0.5 effS "<<rejB_05<<" at "<<effS_05<<std::endl;

//calculate KS score
   Double_t ksS=0;
   Double_t ksB=0;
   Double_t KS=0;
   myData->SetCurrentType(TMVA::Types::kTraining);
   TMVA::ResultsClassification* mvaResTrain = dynamic_cast<TMVA::ResultsClassification*>
       (myData->GetResults(myMethod->GetMethodName(),TMVA::Types::kTraining, TMVA::Types::kClassification) );
   TH1* mva_s_tr = (TH1*) mvaRes->GetHist("MVA_TRAIN_S");
   TH1* mva_b_tr = (TH1*) mvaRes->GetHist("MVA_TRAIN_B");
   ksS=mva_s->KolmogorovTest(mva_s_tr);
   ksB=mva_b->KolmogorovTest(mva_b_tr);
   KS=TMath::Min(ksS, ksB);

   std::cout<<"KS Sig: " << ksS << std::endl;
   std::cout<<"KS Bkg: " << ksB << std::endl;
   std::cout<<"KS: "     << KS  << std::endl;
   std::cout<<"ROC: "    << ROC << std::endl;
   std::cout<<"Sep: "    << sep << std::endl;

    *testKS=KS;
   //choose wanted FOM
   std::cout<<"chosen FOM "<<FOMType<<std::endl;
   if(FOMType=="ROCIntegral"){
     *testFOM=ROC;
   }
   else if(FOMType=="Separation"){
     *testFOM=sep;
   }
   else if(FOMType=="rejB_vs_effS_0.1"){
     *testFOM=rejB_01;
   }
   else if(FOMType=="rejB_vs_effS_0.3"){
     *testFOM=rejB_03;
   }
   else if(FOMType=="rejB_vs_effS_0.5"){
     *testFOM=rejB_05;
   }
   else if(FOMType=="Chi2_B_muSB"){
     *testFOM=chi2FOM;
   }
   else{
     std::cout<<"ERROR: Dont know the requested FOM"<<std::endl<<"Implement it !"<<std::endl;
     exit(1);
   }
   std::cout<<"FOM "<<*testFOM<<std::endl;


   //timing studies
   thisTestingTime=thisTimer->RealTime();
   std::cout<<"*******************************************************"<<std::endl;
   std::cout<<"* this Iteration Tree processing time "<<thisTreeTime<<std::endl;
   std::cout<<"* this Iteration Training time "<<thisTrainingTime<<std::endl;
   std::cout<<"* this Iteration Testing  time "<<thisTestingTime<<std::endl;
   std::cout<<"*******************************************************"<<std::endl;
   TimeTreePreparation+=thisTreeTime;
   TimeTraining+=thisTrainingTime;
   TimeTesting+=thisTestingTime;
   delete thisTimer;

// free all the memory
// hopefully nothing is missed

//    delete mvaRes;
//    delete mvaResTrain;
   delete myData;
//    delete mva_s;
//    delete mva_b;
//    std::cout<<"here1"<<std::endl;
//    delete mva_effS;
//    delete mva_effB;
   delete mva_ROCCurve_H;
//    delete mva_s_tr;
//    delete mva_b_tr;
//    std::cout<<"here2"<<std::endl;
   delete factory;
//    std::cout<<"here3"<<std::endl;
   if(UseFixedTrainTestSplitting==0){
   delete signal;
   delete background;
   inputS->Close();
   inputB->Close();
//    std::cout<<"here4"<<std::endl;
   delete inputS;
   delete inputB;
   }
   else{
   delete signalTrain;
   delete backgroundTrain;
   inputSTrain->Close();
   inputBTrain->Close();
//    std::cout<<"here4"<<std::endl;
   delete inputSTrain;
   delete inputBTrain;
   delete signalTest;
   delete backgroundTest;
   inputSTest->Close();
   inputBTest->Close();
//    std::cout<<"here4"<<std::endl;
   delete inputSTest;
   delete inputBTest;   
   }
   outputFile->Close();
   delete outputFile;

 }
    
 
void Particle()
{
  TStopwatch* timerTotal=new TStopwatch();
  timerTotal->Start();

 //set up variables
 std::vector<BDTVar*> InitialVars;
 std::vector<BDTVar*> OtherVars;
 int particleNumber;
 TString Iteration="0";
 TString FOMType="";
 TString SaveTrainingsToTrees="True";
 Double_t KSThreshold=0.0;
 TString FactoryString="";
 TString PrepString="";
 TString SigWeight="";
 TString BkgWeight="";
 TString SignalTreeName="MVATree";
 TString BackgroundTreeName="MVATree";
 Int_t FindBestVariables=1;
 Int_t MaxVariablesInCombination=10;
 Int_t MinVariablesInCombination=6;
 Double_t ImprovementThreshold=1.0;
 int RepeatTrainingNTimes=0;
 TString MethodType="";
 TString MethodString="";
 std::vector<TString> coordNames;
 std::vector<Double_t> coordValues;
 Double_t dumpVal;
 TString dumpName;
 Int_t UseFixedTrainTestSplitting = 0;

  //read Config File
  std::ifstream config("ParticleConfig.txt");
  TString dump="";
  
  int count=0;
  bool readline=true;
  std::cout<<"reading Config"<<std::endl;
  while(readline==true && count<400){
    count++;
    config>>dump;
    if(config.eof())readline=false;
    if(dump=="particleNumber"){
      config>>particleNumber;}
    if(dump=="Iteration"){
      config>>Iteration;}
    if(dump=="FOM"){
      config>>FOMType;}
    if(dump=="SaveTrainingsToTrees"){
      config>>SaveTrainingsToTrees;}
    if(dump=="KSThreshold"){
      config>>KSThreshold;}
    if(dump=="FactoryString"){
      config>>FactoryString;}
    if(dump=="PreparationString"){
      config>>PrepString;}
    if(dump=="SignalWeightExpression"){
      config>>SigWeight;}
    if(dump=="BackgroundWeightExpression"){
      config>>BkgWeight;}
    if(dump=="SignalTreeName"){
      config>>SignalTreeName;}
    if(dump=="BackgroundTreeName"){
      config>>BackgroundTreeName;}
    if(dump=="UseFixedTrainTestSplitting"){
      config>>UseFixedTrainTestSplitting;}
    if(dump=="FindBestVariables"){
      config>>FindBestVariables;}
    if(dump=="MaxVariablesInCombination"){
      config>>MaxVariablesInCombination;}
    if(dump=="ImprovementThreshold"){
      config>>ImprovementThreshold;}
    if(dump=="RepeatTrainingNTimes"){
      config>>RepeatTrainingNTimes;}
    if(dump=="MethodType"){
      config>>MethodType;}
    if(dump=="MethodParameters"){
      config>>MethodString;}
    if(dump=="coord"){
      config>>dumpName;
      config>>dumpVal;
      std::cout<<dump<<" "<<dumpName<<" "<<dumpVal<<std::endl;
      coordNames.push_back(dumpName);
      coordValues.push_back(dumpVal);}
    if(dump=="--InitialVariables--"){
      bool readVars=true;
      while(readVars==true){
        config>>dump;
        if(dump=="--EndInitVars--"){ readVars=false; continue;}
        InitialVars.push_back(new BDTVar);
        InitialVars.back()->name=dump;
      }
    }
    if(dump=="--additionalVariables--"){
      bool readVars=true;
      while(readVars){
        config>>dump;
        if(dump=="--EndAddVars--"){ readVars=false; continue;}
        OtherVars.push_back(new BDTVar);
        OtherVars.back()->name=dump;
      }
    }
  }
  config.close();
  
  std::cout<<"Config File read"<<std::endl;
  std::cout<<"--Initial Variables--"<<std::endl;
  for(int l=0;l<InitialVars.size();l++){std::cout<<InitialVars.at(l)->name<<std::endl;}
  std::cout<<"--AdditionalVariables--"<<std::endl;
  for(int l=0;l<OtherVars.size();l++){std::cout<<OtherVars.at(l)->name<<std::endl;}
  std::cout<<"--Method--"<<std::endl;
  std::cout<<MethodType<<std::endl;
  std::cout<<MethodString<<std::endl;
//done reading config

  
  Double_t KS=1.0;
  Double_t FOM=999.9;
  Double_t BestKS=0.0;
  Double_t BestFOM=0.0;
  Double_t SecBestKS=0.0;
  Double_t SecBestFOM=0.0;
  Double_t bufferKS=1.0;
  Double_t bufferFOM=999.9;

  std::vector<BDTVar*> BestVars;
  std::vector<BDTVar*> SecBestVars;
  std::vector<BDTVar*> WorstVars;
  std::vector<BDTVar*> UsedVars;
  std::vector<BDTVar*> UnusedVars;
  std::vector<BDTVar*> BestUnusedVars;

  //do initial training
  KS=1.0;
  FOM=999.9;
  for(int nn=0;nn<RepeatTrainingNTimes+1;nn++)
  {
    DoTraining(InitialVars,OtherVars,FOMType,FactoryString,PrepString,SigWeight,BkgWeight,SignalTreeName,BackgroundTreeName,MethodType,MethodString, particleNumber, &bufferFOM, &bufferKS,UseFixedTrainTestSplitting);
    if(bufferKS  < KS ){ KS  = bufferKS;  }
    if(bufferFOM < FOM){ FOM = bufferFOM; }
  }

  std::cout<<"Inital "<<RepeatTrainingNTimes+1<<" trainigns KS, FOM "<<KS<<" "<<FOM<<std::endl;

  //check different Variable Combinations
  int nUsedVars=InitialVars.size();

  for(std::vector<BDTVar*>::iterator itVar=InitialVars.begin(); itVar!=InitialVars.end();++itVar){
    BDTVar* iVar=*itVar;
    BestVars.push_back(iVar);
    UsedVars.push_back(iVar);
  }

  for(std::vector<BDTVar*>::iterator itVar=OtherVars.begin(); itVar!=OtherVars.end();++itVar){
    BDTVar* iVar=*itVar;
    UnusedVars.push_back(iVar);
    WorstVars.push_back(iVar);
    BestUnusedVars.push_back(iVar);
  }

  if((KS < 1.0) and (KS > KSThreshold) and (FOM >= BestFOM))
  {
    BestFOM=FOM;
    BestKS=KS;
    BestVars.clear();
    for(std::vector<BDTVar*>::iterator itVar=InitialVars.begin(); itVar!=InitialVars.end();++itVar){
      BDTVar* iVar=*itVar;
      BestVars.push_back(iVar);
    }
  }

  std::cout<<"Initial KS , FOM "<<BestKS<<", "<<BestFOM<<std::endl<<std::endl;

  if(bool(FindBestVariables))
  {
    std::cout<<"Removing each Variable"<<std::endl;
    // remove worst Variable
    BDTVar* worstVar = new BDTVar;
    for(int k=0;k<nUsedVars;k++){
      BDTVar* testVar = new BDTVar;
      testVar->name=UsedVars.at(k)->name;
      UsedVars.erase(UsedVars.begin()+k);
      UnusedVars.push_back(testVar);
      
      KS=1.0;
      FOM=999.9;
      for(int nn=0;nn<RepeatTrainingNTimes+1;nn++){
      std::cout<<"Training Nr. "<<nn<<std::endl;  
      DoTraining(UsedVars, UnusedVars,FOMType,FactoryString,PrepString,SigWeight,BkgWeight,SignalTreeName,BackgroundTreeName,MethodType,MethodString, particleNumber, &bufferFOM, &bufferKS,UseFixedTrainTestSplitting);
      if(bufferKS<KS)KS=bufferKS;
      if(bufferFOM<FOM)FOM=bufferFOM;
      }
      std::cout<<"after "<<RepeatTrainingNTimes+1<<" trainigns KS, FOM "<<KS<<" "<<FOM<<std::endl;
      
      if(KS>KSThreshold and FOM>=BestFOM){
        BestFOM=FOM;
        BestKS=KS;
        BestVars.clear();
        for(int l=0;l<UsedVars.size();l++){
          BestVars.push_back(new BDTVar);
          BestVars.at(l)->name=UsedVars.at(l)->name; 
        }
        BestUnusedVars.clear();
        for(int l=0;l<UnusedVars.size();l++){
          BestUnusedVars.push_back(new BDTVar);
          BestUnusedVars.at(l)->name=UnusedVars.at(l)->name; 
        }
        
      }
//       std::cout<<FOM<<std::endl;
      if(KS>0.0 and FOM>SecBestFOM){
        SecBestFOM=FOM;
        SecBestKS=KS;
        SecBestVars.clear();
        for(int l=0;l<UsedVars.size();l++){
          SecBestVars.push_back(new BDTVar);
          SecBestVars.at(l)->name=UsedVars.at(l)->name; 
        }
        worstVar->name=testVar->name;
       }
      
       UsedVars.insert(UsedVars.begin()+k, testVar);
       UnusedVars.pop_back();
    }
    std::cout<<"worst Variable "<<worstVar->name<<std::endl;
    std::cout<<BestKS<<" "<<BestFOM<<std::endl;
    if(BestFOM==0.0){
      std::cout<<"here"<<std::endl;
      BestVars.clear();
      for(int l=0;l<SecBestVars.size();l++){
        BestVars.push_back(new BDTVar);
        BestVars.at(l)->name=SecBestVars.at(l)->name; 
      }
      BestUnusedVars.clear();
      for(int l=0;l<OtherVars.size();l++){
//         if(!OtherVars.at(l)->name.EqualTo(worstVar->name)){
          BestUnusedVars.push_back(new BDTVar);
          BestUnusedVars.back()->name=OtherVars.at(l)->name;
//         }
      }
      BestUnusedVars.push_back(new BDTVar);
      BestUnusedVars.back()->name=worstVar->name;
    }

    //add one more Variable to improve result
    BDTVar* addedVar=new BDTVar;
    BDTVar* testVar=new BDTVar;
    
    if(BestVars.size()<MaxVariablesInCombination || BestVars.size()<MinVariablesInCombination){
    
    std::cout<<"adding one variable"<<std::endl;
    UsedVars.clear();
    for(int k=0;k<SecBestVars.size();k++){
      UsedVars.push_back(new BDTVar);
      UsedVars.at(k)->name=SecBestVars.at(k)->name; 
    }
    OtherVars.push_back(new BDTVar);
    OtherVars.back()->name=worstVar->name;
    for(int k=0; k<OtherVars.size();k++){
      testVar->name=OtherVars.at(k)->name;
      UsedVars.push_back(testVar);
      OtherVars.erase(OtherVars.begin()+k);
      UnusedVars.clear();
      for(int l=0;l<OtherVars.size();l++){
        UnusedVars.push_back(new BDTVar);
        UnusedVars.at(l)->name=OtherVars.at(l)->name; 
      }
       
      KS=1.0;
      FOM=999.9;
      for(int nn=0;nn<RepeatTrainingNTimes+1;nn++){
      std::cout<<"Training Nr. "<<nn<<std::endl; 
      DoTraining(UsedVars, UnusedVars,FOMType,FactoryString,PrepString,SigWeight,BkgWeight,SignalTreeName,BackgroundTreeName,MethodType,MethodString, particleNumber, &bufferFOM, &bufferKS,UseFixedTrainTestSplitting);
      if(bufferKS<KS)KS=bufferKS;
      if(bufferFOM<FOM)FOM=bufferFOM;
      }
      std::cout<<"after "<<RepeatTrainingNTimes+1<<" trainigns KS, FOM "<<KS<<" "<<FOM<<std::endl;

      
      if(KS>KSThreshold and FOM>=ImprovementThreshold*BestFOM){
        BestFOM=FOM;
        BestKS=KS;
        BestVars.clear();
        for(int l=0;l<UsedVars.size();l++){
          BestVars.push_back(new BDTVar);
          BestVars.at(l)->name=UsedVars.at(l)->name; 
        }
        BestUnusedVars.clear();
        for(int l=0;l<UnusedVars.size();l++){
          BestUnusedVars.push_back(new BDTVar);
          BestUnusedVars.at(l)->name=UnusedVars.at(l)->name; 
        }
        addedVar->name=testVar->name;
        std::cout<<addedVar->name<<" "<<testVar->name<<std::endl;
      }
      
      OtherVars.insert(OtherVars.begin()+k, new BDTVar);
      OtherVars.at(k)->name=testVar->name;
      UsedVars.pop_back();
      
    }
//     std::cout<<addedVar->name<<" "<<testVar->name<<std::endl;

    std::cout<<"Variable added "<<addedVar->name<<std::endl;
    std::cout<<BestKS<<" "<<BestFOM<<std::endl;
    }
    //add another Variable to improve result
    
    
    //check if the previous step added a avriable
    bool dosecondTry=true;
    if(addedVar->name==""){
      std::cout<<"no improvement by adding variables"<<std::endl;
      dosecondTry=false;
    }
    if(BestVars.size()>=MaxVariablesInCombination){
      std::cout<<"maximal number of Variables to use reached "<<BestVars.size()<<std::endl;
      dosecondTry=false;
    }
    if(BestVars.size()<MinVariablesInCombination){
      std::cout<<" below the minimal number of Variables to use "<<BestVars.size()<<std::endl;
      dosecondTry=true;
    }
    if(dosecondTry){
    std::cout<<"add another"<<std::endl;
    BDTVar* secaddedVar=new BDTVar;
    UsedVars.clear();
    std::cout<<BestVars.size()<<std::endl;
    std::cout<<OtherVars.size()<<std::endl;
    
    

    for(int k=0;k<BestVars.size();k++){
      UsedVars.push_back(new BDTVar);
      UsedVars.at(k)->name=BestVars.at(k)->name; 
    }
    UnusedVars.clear();
//     std::cout<<"step 0"<<std::endl;

    for(int k=0;k<OtherVars.size();k++){
      std::cout<<"OtherVars "<<OtherVars.at(k)->name<<std::endl;

      if(!OtherVars.at(k)->name.EqualTo(addedVar->name)){
      UnusedVars.push_back(new BDTVar);
      UnusedVars.back()->name=OtherVars.at(k)->name;
      }
//       if(OtherVars.at(k)->name.EqualTo(addedVar->name))UnusedVars.pop_back(); 
    }
//     std::cout<<"step 1"<<std::endl;
    for(int k=0; k<UnusedVars.size();k++){
//       BDTVar* testVar;
      testVar->name=UnusedVars.at(k)->name;
      UsedVars.push_back(testVar);
      UnusedVars.erase(UnusedVars.begin()+k);
//     std::cout<<"step 2"<<std::endl;
         
      KS=1.0;
      FOM=999.9;
      for(int nn=0;nn<RepeatTrainingNTimes+1;nn++){
      std::cout<<"Training Nr. "<<nn<<std::endl;  
      DoTraining(UsedVars, UnusedVars,FOMType,FactoryString,PrepString,SigWeight,BkgWeight,SignalTreeName,BackgroundTreeName,MethodType,MethodString, particleNumber, &bufferFOM, &bufferKS,UseFixedTrainTestSplitting);
      if(bufferKS<KS)KS=bufferKS;
      if(bufferFOM<FOM)FOM=bufferFOM;
      }
      std::cout<<"after "<<RepeatTrainingNTimes+1<<" trainigns KS, FOM "<<KS<<" "<<FOM<<std::endl;

      
      if((KS>KSThreshold and FOM>=ImprovementThreshold*BestFOM) or (FOM>BestFOM and BestVars.size()<MinVariablesInCombination)){
        BestFOM=FOM;
        BestKS=KS;
        BestVars.clear();
        for(int l=0;l<UsedVars.size();l++){
          BestVars.push_back(new BDTVar);
          BestVars.at(l)->name=UsedVars.at(l)->name; 
        }
        WorstVars.clear();
        for(int l=0;l<UnusedVars.size();l++){
          WorstVars.push_back(new BDTVar);
          WorstVars.at(l)->name=UnusedVars.at(l)->name; 
        }
        BestUnusedVars.clear();
        for(int l=0;l<UnusedVars.size();l++){
          BestUnusedVars.push_back(new BDTVar);
          BestUnusedVars.at(l)->name=UnusedVars.at(l)->name; 
        }
        secaddedVar->name=testVar->name;
        std::cout<<secaddedVar->name<<" "<<testVar->name<<std::endl;

      }
      
      UnusedVars.insert(UnusedVars.begin()+k, new BDTVar);
      UnusedVars.at(k)->name=testVar->name;
      UsedVars.pop_back();
      
    }
    std::cout<<"second Variable added "<<secaddedVar->name<<std::endl;
    std::cout<<BestKS<<" "<<BestFOM<<std::endl;
    }
    else{
      std::cout<<"return to original Vars"<<std::endl;
//       BestVars.clear();
//       for(int l=0;l<SecBestVars.size();l++){
//         BestVars.push_back(new BDTVar);
//         BestVars.at(l)->name=SecBestVars.at(l)->name; 
//       }
      WorstVars.clear();
      for(int l=0;l<OtherVars.size();l++){
        WorstVars.push_back(new BDTVar);
        WorstVars.at(l)->name=OtherVars.at(l)->name; 
      }  
    }

    std::cout<<"Best FOM "<<BestFOM<<" "<<BestKS<<std::endl;

    for(int k=0;k<BestVars.size();k++){
      std::cout<<BestVars.at(k)->name<<std::endl; 
    }

//    TString ROCFileName="ROC_Particle";
//    ROCFileName+=particleNumber;
//    ROCFileName+="_Iteration";
//    ROCFileName+=Iteration;

    KS=1.0;
    FOM=999.9;
    for(int nn=0;nn<RepeatTrainingNTimes+1;nn++)
    {
      DoTraining(BestVars, WorstVars,FOMType,FactoryString,PrepString,SigWeight,BkgWeight,SignalTreeName,BackgroundTreeName,MethodType,MethodString, particleNumber, &bufferFOM, &bufferKS,UseFixedTrainTestSplitting);
      if(bufferKS<KS)KS=bufferKS;
      if(bufferFOM<FOM)FOM=bufferFOM;
    }

    std::cout<<"Final "<<RepeatTrainingNTimes+1<<" trainigns KS, FOM "<<KS<<" "<<FOM<<std::endl;

    BestFOM = FOM;
    BestKS = KS;
  }

  std::ofstream result("ParticleResult.txt", std::ofstream::trunc);
  result<<"BestFOM "<<BestFOM<<std::endl;
  result<<"KSScore "<<BestKS<<std::endl;
  result<<"MethodString "<<MethodString<<std::endl;
//   result<<BestVars.size()<<std::endl;
//   result<<BestUnusedVars.size()<<std::endl;
  for(int k=0;k<BestVars.size();k++){
    std::cout<<"UsedVar "<<BestVars.at(k)->name<<std::endl; 
    result<<"UsedVar "<<BestVars.at(k)->name<<std::endl; 
  }
//   result<<"UnusedVars"<<std::endl;
  for(int k=0;k<BestUnusedVars.size();k++){
    std::cout<<"UnusedVars "<<BestUnusedVars.at(k)->name<<std::endl;
    result<<"UnusedVars "<<BestUnusedVars.at(k)->name<<std::endl; 
  }
  result.close();

  TimeTotal = timerTotal->RealTime();

  std::cout<<"*******************************************************"<<std::endl;
  std::cout<<"* Total Time: "<<TimeTotal<<std::endl;
  std::cout<<"* Tree processing time : "<<TimeTreePreparation<< " "<<TimeTreePreparation/TimeTotal*100.0<<"%"<<std::endl;
  std::cout<<"* Training time : "<<TimeTraining<< " "<<TimeTraining/TimeTotal*100.0<<"%"<<std::endl;
  std::cout<<"* Testing time : "<<TimeTesting<< " "<<TimeTesting/TimeTotal*100.0<<"%"<<std::endl;
  std::cout<<"*******************************************************"<<std::endl;
}


# ifndef __CINT__
int main()
{
  Particle();
  return 0;
}
# endif
