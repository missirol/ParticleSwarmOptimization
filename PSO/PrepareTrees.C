#include "TreeHeader.h"
#include <fstream>

void PrepareTrees(string configName=""){
string OutPath="InitData/";
string SignalFile;
string SignalTree;
string BackgroundFile;
string BackgroundTree;
string Selection="";

// read config file
std::ifstream config(configName.c_str());
bool readline=true;
std::string linebuffer;
std::string searchedString;

while(readline){
/*  config>>linebuffer;*/
  std::getline(config,linebuffer);
  if(config.eof())readline=false;
  if(linebuffer.find("#")!=std::string::npos)continue;
//   if(linebuffer=="")std::cout<<"empty"<<std::endl;
  
  searchedString="SourceSignalFile=";
  if(linebuffer.find(searchedString)!=std::string::npos){
    SignalFile=linebuffer.substr(searchedString.length());
    std::cout<<"found SignalFile"<<std::endl;
  }
  searchedString="SourceSignalTree=";
  if(linebuffer.find(searchedString)!=std::string::npos){
    SignalTree=linebuffer.substr(searchedString.length());
  }
  searchedString="SourceBackgroundFile=";
  if(linebuffer.find(searchedString)!=std::string::npos){
    BackgroundFile=linebuffer.substr(searchedString.length());
    std::cout<<"found SignalFile"<<std::endl;
  }
  searchedString="SourceBackgroundTree=";
  if(linebuffer.find(searchedString)!=std::string::npos){
    BackgroundTree=linebuffer.substr(searchedString.length());
  }
  searchedString="SelectionString=";
  if(linebuffer.find(searchedString)!=std::string::npos){
    Selection=linebuffer.substr(searchedString.length());
  }
    
  //look for InitialVariables
  searchedString="InitialVariables";
  if(linebuffer.find(searchedString)!=std::string::npos){
    bool readingVars=true;
//     std::cout<<"InitialVariables:"<<std::endl;
    while(readingVars){
      std::getline(config,linebuffer);
      if(linebuffer.find("#")!=std::string::npos || linebuffer=="")continue;
      if(linebuffer.find("EndVariables")!=std::string::npos || config.eof()){
        readingVars=false; 
        continue;
        }
      string name,type,size,expression;
      size_t id1, id2;
      id1=linebuffer.find("\"");
      id2=linebuffer.find("\"",id1+1);
      name=linebuffer.substr(id1+1,id2-id1-1);
      id1=linebuffer.find("\"",id2+1);
      id2=linebuffer.find("\"",id1+1);
      type=linebuffer.substr(id1+1,id2-id1-1);
      id1=linebuffer.find("\"",id2+1);
      id2=linebuffer.find("\"",id1+1);
      size=linebuffer.substr(id1+1,id2-id1-1);
      id1=linebuffer.find("\"",id2+1);
      id2=linebuffer.find("\"",id1+1);
      expression=linebuffer.substr(id1+1,id2-id1-1);
//       std::cout<<"Did Variable "<<name<<" "<<type<<" "<<size<<" "<<expression<<std::endl;
//       if(size=="")std::cout<<"zero"<<std::endl;
      AddVar(name,type,size,expression );
    }
  }
        
  searchedString="AdditionalVariables";
  if(linebuffer.find(searchedString)!=std::string::npos){
    bool readingVars=true;
//     std::cout<<"AdditionalVariables:"<<std::endl;
    while(readingVars){
      std::getline(config,linebuffer);
      if(linebuffer.find("#")!=std::string::npos || linebuffer=="")continue;
      if(linebuffer.find("EndVariables")!=std::string::npos || config.eof()){
        readingVars=false; 
        continue;
        }
      string name,type,size,expression;
      size_t id1, id2;
      id1=linebuffer.find("\"");
      id2=linebuffer.find("\"",id1+1);
      name=linebuffer.substr(id1+1,id2-id1-1);
      id1=linebuffer.find("\"",id2+1);
      id2=linebuffer.find("\"",id1+1);
      type=linebuffer.substr(id1+1,id2-id1-1);
      id1=linebuffer.find("\"",id2+1);
      id2=linebuffer.find("\"",id1+1);
      size=linebuffer.substr(id1+1,id2-id1-1);
      id1=linebuffer.find("\"",id2+1);
      id2=linebuffer.find("\"",id1+1);
      expression=linebuffer.substr(id1+1,id2-id1-1);
//       std::cout<<"Did Variable "<<name<<" "<<type<<" "<<size<<" "<<expression<<std::endl;
//       if(size=="")std::cout<<"zero"<<std::endl;
      AddVar(name,type,size,expression );
    }
  }

  searchedString="WeightVariables";
  if(linebuffer.find(searchedString)!=std::string::npos){
    bool readingVars=true;
//     std::cout<<"WeightVariables:"<<std::endl;
    while(readingVars){
      std::getline(config,linebuffer);
      if(linebuffer.find("#")!=std::string::npos || linebuffer=="")continue;
      if(linebuffer.find("EndVariables")!=std::string::npos || config.eof()){
        readingVars=false; 
        continue;
        }
      string name,type,size,expression;
      size_t id1, id2;
      id1=linebuffer.find("\"");
      id2=linebuffer.find("\"",id1+1);
      name=linebuffer.substr(id1+1,id2-id1-1);
      id1=linebuffer.find("\"",id2+1);
      id2=linebuffer.find("\"",id1+1);
      type=linebuffer.substr(id1+1,id2-id1-1);
      id1=linebuffer.find("\"",id2+1);
      id2=linebuffer.find("\"",id1+1);
      size=linebuffer.substr(id1+1,id2-id1-1);
      id1=linebuffer.find("\"",id2+1);
      id2=linebuffer.find("\"",id1+1);
      expression=linebuffer.substr(id1+1,id2-id1-1);
//       std::cout<<"Did Variable "<<name<<" "<<type<<" "<<size<<" "<<expression<<std::endl;
//       if(size=="")std::cout<<"zero"<<std::endl;
      AddVar(name,type,size,expression );
    }
  }
}
config.close();


AddSample("Signal",SignalFile,SignalTree,Selection);
AddSample("Background",BackgroundFile,BackgroundTree,Selection);

AddBin("all","all","");
bool flatTrees = false;
bool appendVariables = false;
SetOutFileDir(OutPath);
FillAllTrees(flatTrees,appendVariables);
}
