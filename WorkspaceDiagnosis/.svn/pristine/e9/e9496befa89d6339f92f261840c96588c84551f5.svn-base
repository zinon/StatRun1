
#include "RooMsgService.h"
#include "TStopwatch.h"

#include <string>

void Run(){
	
	TStopwatch sw;	sw.Start();
	
	string tag = "Oct10";
	
	string roofilename = tag + "_corr";

	//load libraries
	if( gSystem->Load("../FitFunction/FitFunction_cpp.so") < 0 ) {cerr<<"Run - unable to load the fit function libary "<<endl; exit(0); } 
	if( gSystem->Load("PullsAndCorrelations_cpp.so") < 0 ) {cerr<<"Run - unable to load the main library "<<endl; exit(0); } 

	
	//umber of cpu's
	SysInfo_t* s = new SysInfo_t(); gSystem->GetSysInfo(s); int Ncpus = s->fCpus;

	
	//the real class
	PullsAndCorrelations c;
	c.SetInputFileName("../../WorkspaceHadHad/swagato_unblinded_random_mu/xml_measurement_hh_combination_125_results/hist2workspace_combined_measurement_hh_combination_125_model.root");
 	c.SetOutputDirectoryName("./results/");
	c.SetOutputFileName(roofilename);
	c.SetWorkspaceName("combined");
	c.SetModelConfigName("ModelConfig");
	c.SetObsDataName("obsData");
// 	c.SetFitConditional();
	c.FitSubChannels();
	c.SetRooMinimizerPrintLvl(0);
	c.SetRooFitMsgLvl(RooFit::WARNING); //DEBUG=0, INFO=1, PROGRESS=2, WARNING=3, ERROR=4, FATAL=5 
	c.SetPOI(0.0);
	c.Minuit2();
	c.MaxFitRetries(3);
	c.Strategy(1);
	c.ParallelProcessing(Ncpus);
// 	c.Minos();
	c.Calculate();
	
	cout<<"Run - Finished: "<<sw.Print()<<endl;
	
	gROOT->ProcessLine(".q");
}
//()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()
