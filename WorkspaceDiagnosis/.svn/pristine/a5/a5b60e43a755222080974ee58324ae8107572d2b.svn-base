#include "RooMsgService.h"
#include "TStopwatch.h"

#include <string>

string rmSpaces(const string &str);

void RunFitAndPlot(){


  if( gSystem->Load("../FitFunction/FitFunction_cpp.so") < 0 ) {cerr<<"RunNLLprofiler - unable to load the fit function libary "<<endl; exit(0); }
  if( gSystem->Load("FitAndPlot_C.so") < 0 ) {cerr<<"FitAndPlot - unable to load the nll libary "<<endl; exit(0); }

  TStopwatch sw;
  sw.Start();

  FitAndPlot();

  gROOT->ProcessLine(".q");
}
