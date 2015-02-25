#include "../RooHeaders/RooHeaders.h"
#include "../FitFunction/FitFunction.h"

#include <map>
#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include "TFile.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TString.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TStyle.h"


using namespace std;

#include <iostream>
#include <sstream>
#include <iomanip>

void Run(const string &, const string &,const string &, const string &, const string &, bool, double, int, double, bool);


RooDataSet* makeAsimovData(ModelConfig  *mc, RooWorkspace *w, double mu_val = 1, bool fluctuateData = false, string* mu_str = NULL);
void unfoldConstraints(RooArgSet& initial, RooArgSet& final, RooArgSet& obs, RooArgSet& nuis, int& counter);

void ComputePulls();

void ComputePulls(){

  string file = "../workspaces/current/output_unblinded_tes_mfs_sym/xml_measurement_hh_combination_125_results/hist2workspace_combined_measurement_hh_combination_125_model.root";
  bool kConditional = false;
  double mu =1.;
  int DefaultStrategy = 1;
  double LumiRelError = 0.03;
  bool Minos = false;
  Run(file, "results", "combined", "ModelConfig", "obsData", kConditional, mu, DefaultStrategy, LumiRelError, Minos);

}


void Run(const string &RooFileName, const string & OutDirName,const string &WorkspaceName, const string &ModelConfigName, const string &ObsDataName, bool kCond, double mu,
         int DefaultStrategy, double LumiRelError, bool kMinos )
{

  bool kPrintFitResult = false;

  bool doAsimov = true;
  double mu_Asimov = 1.;

  //output txt file - open a file in write mode.
  ofstream outfilePulls; outfilePulls.open("pulls.dat");
  ofstream outfileFit; outfileFit.open("fit.dat");

  //roofit verbosity
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);//enum MsgLevel { DEBUG=0, INFO=1, PROGRESS=2, WARNING=3, ERROR=4, FATAL=5 } ; RooFit::WARNING

  //accellerate science
  SysInfo_t* s = new SysInfo_t(); gSystem->GetSysInfo(s); int Ncpus = s->fCpus; cout<<"CPUs "<<Ncpus<<endl;

  //initial snapshot
  string snapName = "snapshot_paramsVals_initial";

  //input roofile
  TFile *f = TFile::Open(RooFileName.c_str());
  if(!f) { cerr << "Cannot load file " << RooFileName <<endl; return;}

  //obtain roo-workspace
  RooWorkspace *w = (RooWorkspace*) f->Get(WorkspaceName.c_str());
  if(!w){ cerr<<"Cannot load workspace "<<WorkspaceName<<endl; return; }
  w->SetName("w"); w->SetTitle("w");

  ModelConfig  *mc = (ModelConfig*) w->obj(ModelConfigName.c_str());

  RooAbsData   *data = w->data(ObsDataName.c_str());

  //asimov
  if (!data || doAsimov  ){
    if ( doAsimov )
      data = makeAsimovData(mc, w, mu_Asimov);
    else
      data = makeAsimovData(mc, w, mu_Asimov, true); //fluctuated
  }

  //save snapshot before any fit
  RooSimultaneous *pdf = (RooSimultaneous*) w->pdf("simPdf");

  //get parameters
  RooArgSet* params = (RooArgSet*) pdf->getParameters(*data);

  //save initial snapshot
  if(!w->loadSnapshot(snapName.c_str()) )
    w->saveSnapshot(snapName.c_str(), *params);
  else
    cout << " Snapshot '"+snapName+"' already exists in  workspace --> will not be overwritten" << endl;

  if(!data){ w->Print(); cerr << "Data was not found" <<endl; return; }

  if(!mc){ w->Print(); cerr << "ModelConfig was not found" <<endl; return; }

  //get nominal NP values
  TIterator *it = mc->GetNuisanceParameters()->createIterator();
  RooRealVar *var = NULL;
  std::map<string, double> map_NP;
  while ((var = (RooRealVar*)it->Next()) != NULL){
    double NPvalue = (double)var->getVal();
    string NPname( var->GetName());
    map_NP.insert( std::pair<std::string,double>(NPname, NPvalue) );
  }
  cout<<"NP : "<<map_NP.size()<<endl;

  // Put all parameters to their initial values
  if(!w->loadSnapshot(snapName.c_str())) {
    cout << "Cannot load snapshot " << snapName << endl; return;
  }

  //poi
  RooRealVar * firstPOI = dynamic_cast<RooRealVar*>(mc->GetParametersOfInterest()->first());
  firstPOI->setVal(mu);

  // Get the RooSimultaneous PDF
  RooSimultaneous *simPdf = (RooSimultaneous*)(mc->GetPdf());

  if(kCond) firstPOI->setConstant();

  //fit settings
  ROOT::Math::MinimizerOptions::SetDefaultStrategy(DefaultStrategy);
  int Minuit_Status = -123;
  int Hesse_Status = -123;
  double EDM = -123;

  //   gSystem->AddIncludePath( (" -I../RooHeaders/") );
  if( gSystem->Load("../FitFunction/FitFunction_cpp.so") < 0 ) {cerr<<"Unable to load the fit function libary "<<endl; return; }

  //fit function
  FitFunction *fit_function = new FitFunction(/*print fit pdf model*/false, /*print fit pdf init params*/false, /*RooMinimizer print lvl*/-1);
  //MINUIT internal printing level: printlevel : -1  quiet (also suppress all warnings), 0  normal, 1  verbose
  //http://root.cern.ch/root/html/TMinuit.html#TMinuit:SetPrintLevel
  //http://root.cern.ch/root/html/TMinuit.html#TMinuit:SetPrintLevel

  if(!fit_function){ cerr<<"NLLprofiler::SetProperties - couldn't create the fit PDF class object"<<endl; return;}

  //global fit
  RooFitResult  *fitresGlobal = fit_function->FitPDF( mc, simPdf, data, Minuit_Status, Hesse_Status, EDM, /*minimizer*/"Minuit2", kMinos, /*max fit retries*/3, /*n cpus*/ 0);

  if(kPrintFitResult) fitresGlobal->Print();

  outfileFit << "global Minuit-status "<<Minuit_Status <<" Hesse-status "<<Hesse_Status<<" EDM "<<EDM<<endl;

  //TString FitName = "GlobalFit_" + TS_IsConditionnal + "_mu";
  //FitName += mu;
  //AllFitResults_map[FitName] = fitresGlobal;
  //AllFitStatus_map[FitName] = status;

  const RooArgSet *ParaGlobalFit = mc->GetNuisanceParameters();
  w->saveSnapshot("snapshot_paramsVals_GlobalFit", *ParaGlobalFit);

  double muhat = firstPOI->getVal();
  firstPOI->setConstant(kFALSE);

  if (kCond)
    cout << "Global - Conditionnal fit, mu is fixed at " << mu << endl;
  else
    cout << "Global - Unconditionnal fit, mu is fitted" << endl;

  //fitresGlobal->Print("v");

  outfileFit << "global mu-" << (kCond ? "fixed " : "best ") << firstPOI->getVal() << " " << firstPOI->getErrorLo() << " " << firstPOI->getErrorHi() << endl;


  TIterator* itr = mc->GetNuisanceParameters()->createIterator();
  while( (var = (RooRealVar*) itr->Next()) ){

    // Do not consider nuisance parameter being not associated to syst
    string varname = (string) var->GetName();
    if ((varname.find("gamma_stat")!=string::npos)) continue;

    double pull  = var->getVal() / 1.0; // GetValue() return value in unit of sigma
    double errorHi = var->getErrorHi() / 1.0;
    double errorLo = var->getErrorLo() / 1.0;

    if(strcmp(var->GetName(),"Lumi")==0){
      pull  = (var->getVal() - w->var("nominalLumi")->getVal() ) / (w->var("nominalLumi")->getVal() * LumiRelError );
      errorHi = var->getErrorHi() / (w->var("nominalLumi")->getVal() * LumiRelError);
      errorLo = var->getErrorLo() / (w->var("nominalLumi")->getVal() * LumiRelError);
    }

    TString vname=var->GetName();
    vname.ReplaceAll("alpha_","");
    vname.ReplaceAll("ATLAS_","");
    vname.ReplaceAll("Lumi","Luminosity");

    outfilePulls << "global "<< vname << " " << pull << " " << errorHi << " " << errorLo <<endl;
  }

  //============================================================================================

  // Put all parameters to their initial values
  if(!w->loadSnapshot(snapName.c_str())) {
    cout << "Cannot load snapshot " << snapName << endl; return;
  }

  //
  RooRealVar *_firstPOI = dynamic_cast<RooRealVar*>(mc->GetParametersOfInterest()->first());
  _firstPOI->setVal(mu);


  // Get the RooSimultaneous PDF
  RooSimultaneous *_simPdf = (RooSimultaneous*)(mc->GetPdf());

  RooCategory* channelCat = (RooCategory*) (&_simPdf->indexCat()); //carefull here: _simPdf
  TIterator *iter = channelCat->typeIterator() ;
  RooCatType *tt  = NULL;
  while((tt=(RooCatType*) iter->Next()) ){
    TString channel_Str(tt->GetName());
    channel_Str.ReplaceAll("channel", "");
    channel_Str.ReplaceAll("125", "");
    channel_Str.ReplaceAll("_", "");
    string channel_str = channel_Str.Data();

    // Get pdf and datset associated to the studied channel
    RooAbsPdf  *pdf_tmp  = simPdf->getPdf( tt->GetName() );
    RooArgSet  *obs_tmp  = pdf_tmp->getObservables( *mc->GetObservables() ) ;
    RooAbsData *data_tmp = data->reduce(Form("%s==%s::%s", channelCat->GetName(), channelCat->GetName(), tt->GetName()));
    RooRealVar *obs = ((RooRealVar*) obs_tmp->first());

    // Fit
    if (kCond) firstPOI->setConstant();
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(DefaultStrategy);

    //fit settings
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(DefaultStrategy);
    int _Minuit_Status = -123;
    int _Hesse_Status = -123;
    double _EDM = -123;

    //fit function
    //FitFunction *fit_function = new FitFunction(/*print fit pdf model*/false, /*print fit pdf init params*/false, /*RooMinimizer print lvl*/-1);


    if(!fit_function){ cerr<<"NLLprofiler::SetProperties - couldn't create the fit PDF class object in "<<channel_str<<endl; return;}

    //channel fit
    RooFitResult  *fitresChannel = fit_function->FitPDF( mc, pdf_tmp, data_tmp, _Minuit_Status, _Hesse_Status, _EDM, /*minimizer*/"Minuit2", kMinos, /*max fit retries*/3, /*n cpus*/ 0);

    if(kPrintFitResult) fitresChannel->Print();
    //  TH2D *h2Dcorrelation = (TH2D*) fitres->correlationHist(cname1);
    outfileFit << channel_str << " Minuit-status "<< _Minuit_Status <<" Hesse-status "<< _Hesse_Status<<" EDM "<< _EDM<<endl;

    if (kCond)
      cout <<channel_str<< " - Conditionnal fit, mu is fixed at " << mu << endl;
    else
      cout <<channel_str<< " - Unconditionnal fit, mu is fitted" << endl;

    double _muhat = firstPOI->getVal();

    _firstPOI->setConstant(kFALSE);

    outfileFit << channel_str<<" mu-" << (kCond ? "fixed " : "best ") << _firstPOI->getVal() << " " << _firstPOI->getErrorLo() << " " << _firstPOI->getErrorHi() << endl;

    TIterator* Itr = mc->GetNuisanceParameters()->createIterator();
    while( (var = (RooRealVar*) Itr->Next()) ){

      //Skip nuisance parameters being not associated to systematic uncertainties
      string _varname = (string) var->GetName();
      if ( (_varname.find("gamma_stat")!=string::npos) ) continue;

      double _pull  = var->getVal() / 1.0 ; // GetValue() return value in unit of sigma
      double _errorHi = var->getErrorHi() / 1.0;
      double _errorLo = var->getErrorLo() / 1.0;

      if(strcmp(var->GetName(),"Lumi")==0){
        _pull  = (var->getVal() - w->var("nominalLumi")->getVal() ) / (w->var("nominalLumi")->getVal() * LumiRelError );
        _errorHi = var->getErrorHi() / (w->var("nominalLumi")->getVal() * LumiRelError);
        _errorLo = var->getErrorLo() / (w->var("nominalLumi")->getVal() * LumiRelError);
      }

      TString _vname=var->GetName();
      _vname.ReplaceAll("alpha_","");
      _vname.ReplaceAll("ATLAS_","");
      _vname.ReplaceAll("Lumi","Luminosity");

      outfilePulls << channel_str<<" "<< _vname << " " << _pull << " " << _errorHi << " " << _errorLo <<endl;
    }//NP

  }//category

  outfilePulls.close();
  outfileFit.close();
}

// --------------------------------------------------------------------------------------------------------------------

RooDataSet* makeAsimovData(ModelConfig  *mc, RooWorkspace *w, double mu_val, bool fluctuateData, string* mu_str){


  ////////////////////
  cout<<"Making asimov data ... "<<endl;
  ////////////////////
  RooAbsPdf* combPdf = mc->GetPdf();

  int _printLevel = 1;

  stringstream muStr;
  muStr << setprecision(5);
  muStr << "_" << mu_val;
  if (mu_str) *mu_str = muStr.str();
  cout<<muStr<<endl;

  stringstream muStrProf;

  RooRealVar* mu = (RooRealVar*)mc->GetParametersOfInterest()->first();//w->var("mu");
  mu->setVal(mu_val);

  RooArgSet mc_obs = *mc->GetObservables();
  RooArgSet mc_globs = *mc->GetGlobalObservables();
  RooArgSet mc_nuis = *mc->GetNuisanceParameters();

  //pair the nuisance parameter to the global observable
  RooArgSet mc_nuis_tmp = mc_nuis;
  RooArgList nui_list("ordered_nuis");
  RooArgList glob_list("ordered_globs");
  RooArgSet constraint_set_tmp(*combPdf->getAllConstraints(mc_obs, mc_nuis_tmp, false));
  RooArgSet constraint_set;
  int counter_tmp = 0;
  unfoldConstraints(constraint_set_tmp, constraint_set, mc_obs, mc_nuis_tmp, counter_tmp);

  TIterator* cIter = constraint_set.createIterator();
  RooAbsArg* arg;
  while ((arg = (RooAbsArg*)cIter->Next())){
    RooAbsPdf* pdf = (RooAbsPdf*)arg;
    if (!pdf) continue;
    TIterator* nIter = mc_nuis.createIterator();
    RooRealVar* thisNui = NULL;
    RooAbsArg* nui_arg;
    while ((nui_arg = (RooAbsArg*)nIter->Next())){
      if (pdf->dependsOn(*nui_arg)){
        thisNui = (RooRealVar*)nui_arg;
        break;
      }
    }
    delete nIter;


    //need this in case the observable isn't fundamental.
    //in this case, see which variable is dependent on the nuisance parameter and use that.
    RooArgSet* components = pdf->getComponents();
    components->remove(*pdf);
    if (components->getSize()){
      TIterator* itr1 = components->createIterator();
      RooAbsArg* arg1;
      while ((arg1 = (RooAbsArg*)itr1->Next())){
        TIterator* itr2 = components->createIterator();
        RooAbsArg* arg2;
        while ((arg2 = (RooAbsArg*)itr2->Next())){
          if (arg1 == arg2) continue;
          if (arg2->dependsOn(*arg1)){
            components->remove(*arg1);
          }
        }
        delete itr2;
      }
      delete itr1;
    }
    if (components->getSize() > 1){
      cout << "ERROR::Couldn't isolate proper nuisance parameter" << endl;
      return NULL;
    }
    else if (components->getSize() == 1){
      thisNui = (RooRealVar*)components->first();
    }

    TIterator* gIter = mc_globs.createIterator();
    RooRealVar* thisGlob = NULL;
    RooAbsArg* glob_arg;
    while ((glob_arg = (RooAbsArg*)gIter->Next())){
      if (pdf->dependsOn(*glob_arg)){
        thisGlob = (RooRealVar*)glob_arg;
        break;
      }
    }
    delete gIter;

    if (!thisNui || !thisGlob){
      cout << "WARNING::Couldn't find nui or glob for constraint: " << pdf->GetName() << endl;
      continue;
    }

    if (_printLevel >= 1) cout << "Pairing nui: " << thisNui->GetName() << ", with glob: " << thisGlob->GetName() << ", from constraint: " << pdf->GetName() << endl;

    nui_list.add(*thisNui);
    glob_list.add(*thisGlob);

  }
  delete cIter;




  //save the snapshots of nominal parameters, but only if they're not already saved
  w->saveSnapshot("tmpGlobs",*mc->GetGlobalObservables());
  w->saveSnapshot("tmpNuis",*mc->GetNuisanceParameters());
  if (!w->loadSnapshot("nominalGlobs")){
    cout << "nominalGlobs doesn't exist. Saving snapshot." << endl;
    w->saveSnapshot("nominalGlobs",*mc->GetGlobalObservables());
  }
  else w->loadSnapshot("tmpGlobs");
  if (!w->loadSnapshot("nominalNuis")){
    cout << "nominalNuis doesn't exist. Saving snapshot." << endl;
    w->saveSnapshot("nominalNuis",*mc->GetNuisanceParameters());
  }
  else w->loadSnapshot("tmpNuis");

  mu->setConstant(0);
  mu->setVal(mu_val);



  //loop over the nui/glob list, grab the corresponding variable from the tmp ws, and set the glob to the value of the nui
  int nrNuis = nui_list.getSize();
  if (nrNuis != glob_list.getSize()){
    cout << "ERROR::nui_list.getSize() != glob_list.getSize()!" << endl;
    return NULL;
  }

  for (int i=0;i<nrNuis;i++){
    RooRealVar* nui = (RooRealVar*)nui_list.at(i);
    RooRealVar* glob = (RooRealVar*)glob_list.at(i);

    cout << "nui: " << nui << ", glob: " << glob << endl;
    cout << "Setting glob: " << glob->GetName() << ", which had previous val: " << glob->getVal() << ", to conditional val: " << nui->getVal() << endl;

    glob->setVal(nui->getVal());
  }

  w->loadSnapshot("nominalGlobs");
  w->loadSnapshot("nominalNuis");

  if (_printLevel >= 1) cout << "Making asimov mu=" <<mu_val<< endl;

  //make the asimov data (snipped from Kyle)
  mu->setVal(mu_val);

  int iFrame=0;

  const char* weightName="weightVar";
  RooArgSet obsAndWeight;
  obsAndWeight.add(*mc->GetObservables());

  RooRealVar* weightVar = NULL;
  if (!(weightVar = w->var(weightName))){
    w->import(*(new RooRealVar(weightName, weightName, 1,0,10000000)));
    weightVar = w->var(weightName);
  }
  obsAndWeight.add(*w->var(weightName));
  w->defineSet("obsAndWeight",obsAndWeight);


  //////////////////////////////////////////////////////
  // MAKE ASIMOV DATA FOR OBSERVABLES
  //////////////////////////////////////////////////////

  RooSimultaneous* simPdf = dynamic_cast<RooSimultaneous*>(mc->GetPdf());

  RooDataSet* asimovData;
  if (!simPdf)
    {
      // Get pdf associated with state from simpdf
      RooAbsPdf* pdftmp = mc->GetPdf();//simPdf->getPdf(channelCat->getLabel()) ;

      // Generate observables defined by the pdf associated with this state
      RooArgSet* obstmp = pdftmp->getObservables(*mc->GetObservables()) ;

      if (_printLevel >= 1) obstmp->Print();

      if( fluctuateData ){
        cout<<"Creating extended datasample"<<endl;
        asimovData = pdftmp->generate(RooArgSet(obsAndWeight),Extended(kTRUE));
      }else{
        asimovData = new RooDataSet(("asimovData"+muStr.str()).c_str(),("asimovData"+muStr.str()).c_str(),RooArgSet(obsAndWeight),WeightVar(*weightVar));

        RooRealVar* thisObs = ((RooRealVar*)obstmp->first());
        double expectedEvents = pdftmp->expectedEvents(*obstmp);
        double thisNorm = 0;
        for(int jj=0; jj<thisObs->numBins(); ++jj){
          thisObs->setBin(jj);

          thisNorm=pdftmp->getVal(obstmp)*thisObs->getBinWidth(jj);
          if (thisNorm*expectedEvents <= 0)
            {
              cout << "WARNING::Detected bin with zero expected events (" << thisNorm*expectedEvents << ") ! Please check your inputs. Obs = " << thisObs->GetName() << ", bin = " << jj << endl;
            }
          if (thisNorm*expectedEvents > 0 && thisNorm*expectedEvents < pow(10.0, 18)) {
            asimovData->add(*mc->GetObservables(), thisNorm*expectedEvents);
          }
        }
      }
      if (_printLevel >= 1){
        asimovData->Print();
        cout <<"sum entries "<<asimovData->sumEntries()<<endl;
      }
      if(asimovData->sumEntries()!=asimovData->sumEntries()){
        cout << "sum entries is nan"<<endl;
        exit(1);
      }

      w->import(*asimovData);

      if (_printLevel >= 1){
        asimovData->Print();
        cout << endl;
      }
    }

  else{
    map<string, RooDataSet*> asimovDataMap;

    //try fix for sim pdf
    RooCategory* channelCat = (RooCategory*)&simPdf->indexCat();
    TIterator* iter = channelCat->typeIterator() ;
    RooCatType* tt = NULL;
    int nrIndices = 0;
    while((tt=(RooCatType*) iter->Next())) {
      nrIndices++;
    }

    for (int i=0;i<nrIndices;i++){
      channelCat->setIndex(i);
      iFrame++;
      // Get pdf associated with state from simpdf
      RooAbsPdf* pdftmp = simPdf->getPdf(channelCat->getLabel()) ;

      // Generate observables defined by the pdf associated with this state
      RooArgSet* obstmp = pdftmp->getObservables(*mc->GetObservables()) ;

      if (_printLevel >= 1){
        obstmp->Print();
        cout << "on type " << channelCat->getLabel() << " " << iFrame << endl;
      }

      RooDataSet* obsDataUnbinned = new RooDataSet(Form("combAsimovData%d",iFrame),Form("combAsimovData%d",iFrame),RooArgSet(obsAndWeight,*channelCat),WeightVar(*weightVar));
      RooRealVar* thisObs = ((RooRealVar*)obstmp->first());
      double expectedEvents = pdftmp->expectedEvents(*obstmp);
      double thisNorm = 0;

      if( fluctuateData ){
        cout<<"Creating extended datasample"<<endl;
        obsDataUnbinned = pdftmp->generate(RooArgSet(obsAndWeight),Extended(kTRUE));
        obsDataUnbinned = pdftmp->generate(RooArgSet(obsAndWeight),Extended(kTRUE));
      }
      else{
        for(int jj=0; jj<thisObs->numBins(); ++jj){
          thisObs->setBin(jj);

          thisNorm=pdftmp->getVal(obstmp)*thisObs->getBinWidth(jj);
          if (thisNorm*expectedEvents > 0 && thisNorm*expectedEvents < pow(10.0, 18)) obsDataUnbinned->add(*mc->GetObservables(), thisNorm*expectedEvents);
        }
      }
      if (_printLevel >= 1){
        obsDataUnbinned->Print();
        cout <<"sum entries "<<obsDataUnbinned->sumEntries()<<endl;
      }
      if(obsDataUnbinned->sumEntries()!=obsDataUnbinned->sumEntries()){
        cout << "sum entries is nan"<<endl;
        exit(1);
      }

      asimovDataMap[string(channelCat->getLabel())] = obsDataUnbinned;

      if (_printLevel >= 1){
        cout << "channel: " << channelCat->getLabel() << ", data: ";
        obsDataUnbinned->Print();
        cout << endl;
      }
    }

    asimovData = new RooDataSet(("asimovData"+muStr.str()).c_str(),
                                ("asimovData"+muStr.str()).c_str(),
                                RooArgSet(obsAndWeight,*channelCat),
                                Index(*channelCat),
                                Import(asimovDataMap),
                                WeightVar(*weightVar));
    w->import(*asimovData);
  }

  //bring us back to nominal for exporting
  w->loadSnapshot("nominalGlobs");

  return asimovData;
}



void unfoldConstraints(RooArgSet& initial, RooArgSet& final, RooArgSet& obs, RooArgSet& nuis, int& counter)
{
  if (counter > 50)
  {
    cout << "ERROR::Couldn't unfold constraints!" << endl;
    cout << "Initial: " << endl;
    initial.Print("v");
    cout << endl;
    cout << "Final: " << endl;
    final.Print("v");
    exit(1);
  }
  TIterator* itr = initial.createIterator();
  RooAbsPdf* pdf;
  while ((pdf = (RooAbsPdf*)itr->Next()))
  {
    RooArgSet nuis_tmp = nuis;
    RooArgSet constraint_set(*pdf->getAllConstraints(obs, nuis_tmp, false));
    //if (constraint_set.getSize() > 1)
    //{
    string className(pdf->ClassName());
    if (className != "RooGaussian" && className != "RooLognormal" && className != "RooGamma" && className != "RooPoisson" && className != "RooBifurGauss")
    {
      counter++;
      unfoldConstraints(constraint_set, final, obs, nuis, counter);
    }
    else
    {
      final.add(*pdf);
    }
  }
  delete itr;
}
