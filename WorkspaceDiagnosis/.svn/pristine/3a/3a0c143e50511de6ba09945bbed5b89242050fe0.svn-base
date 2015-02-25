#include "RooMsgService.h"
#include "TStopwatch.h"

#include <string>
#include <sys/stat.h>

string rmSpaces(const string &str);

void RunNLLprofiler();

void RunNLLprofiler(string NPvar, string rws, string outdir){

  bool dryRun = false;
  
  tag = rmSpaces(outdir);
  rws = rmSpaces(rws);
  NPvar = rmSpaces(NPvar);
  cout << "RunNLLprofiler - scanning " << NPvar <<endl;

  string roofilename = NPvar.size() ? "nll_profiles_" + NPvar : "test";

  if( gSystem->Load("../FitFunction/FitFunction_cpp.so") < 0 ) {cerr<<"RunNLLprofiler - unable to load the fit function libary "<<endl; exit(0); }
  if( gSystem->Load("NLLprofiler_cpp.so") < 0 ) {cerr<<"RunNLLprofiler - unable to load the nll libary "<<endl; exit(0); }


  //umber of cpu's
  SysInfo_t* s = new SysInfo_t(); gSystem->GetSysInfo(s); int Ncpus = s->fCpus;

  TStopwatch sw;
  sw.Start();

  string	 outputdir =  "./results/"+outdir+"/";
  struct stat dst;
  int dstatus = stat(outputdir.c_str(),&dst);
  if( dstatus ){
	  cout<<"\ncreating directory "<<outputdir<<endl;
	  system(("mkdir -p "+outputdir).c_str());
  }
  NLLprofiler c;
  c.SetInputFileName(rws);
  c.SetOutputDirectoryName(outputdir);
  c.SetOutputFileName(roofilename);
  c.SetWorkspaceName("combined");
  c.SetModelConfigName("ModelConfig");
  c.SetObsDataName("obsData");
  //    c.Print_ModelObservables();
  //    c.Print_NuisanceParameters();
  //c.Print_NuisanceParametersList();
  //    c.Print_SubChannels();
  //    c.Print_FitPDF_InitialParams();
  //    c.Print_FitPDF_Model();
  //    c.SetFitConditional();
  c.FitSubChannels();
  c.SetRooMinimizerPrintLvl(-1); //MINUIT internal printing level: printlevel : -1  quiet (also suppress all warnings), 0  normal, 1  verbose http://root.cern.ch/root/html/TMinuit.html#TMinuit:SetPrintLevel  and  http://root.cern.ch/root/html/TMinuit.html#TMinuit:SetPrintLevel 
  c.SetRooFitMsgLvl(RooFit::WARNING); //DEBUG=0, INFO=1, PROGRESS=2, WARNING=3, ERROR=4, FATAL=5
  c.SetPOI(0.0);
  c.Minuit2();
  c.MaxFitRetries(3);
  c.Strategy(1);
  //    c.ParallelProcessing(Ncpus);
  //    c.Minos();
  c.ScanPOI();
  if(NPvar.size()) c.ScanVariable(NPvar);
  c.ScanGreen();
  c.Calculate(!dryRun);
  cout<<"RunNLLprofiler - Finished: "<<sw.Print()<<endl;

  gROOT->ProcessLine(".q");
}
//()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()
void RunNLLprofiler(){
  RunNLLprofiler("","","");
}
string rmSpaces(const string &str){
  string temp;
  for (unsigned int i = 0; i < str.length(); i++)
    if (str[i] != ' ') temp += str[i];

  return temp;
}
