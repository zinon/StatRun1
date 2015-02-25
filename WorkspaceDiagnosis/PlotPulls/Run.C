#include "RooMsgService.h"
#include "TStopwatch.h"

#include <string>

string rmSpaces(const string &str);

void Run(){


  if( gSystem->Load("../FitFunction/FitFunction_cpp.so") < 0 ) {cerr<<"RunNLLprofiler - unable to load the fit function libary "<<endl; exit(0); }
  if( gSystem->Load("ComputePulls_C.so") < 0 ) {cerr<<"FitAndPlot - unable to load the nll libary "<<endl; exit(0); }

  TStopwatch sw;
  sw.Start();

  ComputePulls();

  gROOT->ProcessLine(".q");
}
