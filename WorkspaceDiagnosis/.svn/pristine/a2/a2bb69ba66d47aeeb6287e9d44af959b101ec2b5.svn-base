#include "NLLprofiler.h"

//ClassImp(NLLprofiler);


const string NLLprofiler::m_snapshot_initial_name = "snapshot_paramsVals_initial";
const double NLLprofiler::m_nnl_range_low = -5.0;
const double NLLprofiler::m_nnl_range_high = 5.0;
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
NLLprofiler::NLLprofiler() :
  m_roofit_msg_lvl(RooFit::INFO),
  m_RooMinimizer_print_lvl(1),
  m_outfile_name("test"),
  m_fit_conditional(false),
  m_fit_subchannels(false),
  m_print_mc_obs(false),
  m_print_nps(false),
  m_print_nps_list(false),
  m_print_subchan(false),
  m_strategy(0),
  m_poi(0),
  m_minimizer("Minuit"),
  m_max_fit_retries(1),
  m_n_cpus(0),
  m_minos(false),
  m_print_fit_pdf_init_params(false),
  m_print_fit_pdf_model(false),
  m_scan_variable(""),
  m_scan_poi(false),
  m_scan_green(false),
  m_LumiRelError(0.03),
  m_input_file_name(""),
  m_output_dir_name("./"),
  m_ws_name("combined"),
  m_confmod_name("ModelConfig"),
  m_obsdfata_name("obsData"),
  m_outfile(NULL),
  m_ws_file(NULL),
  m_ws(NULL),
  m_mc(NULL),
  m_data(NULL),
  m_pdf(NULL),
  m_parameters(NULL),
  m_observables(NULL),
  m_firstPOI(NULL)
{

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
NLLprofiler::~NLLprofiler()
{
  m_outfile->Save();
  m_outfile->Close();

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void NLLprofiler::Calculate(bool kRun){

  if (! SetProperties() ){cerr<<"NLLprofiler::Calculate - unable to set properties"<<endl; exit(0); }

  if( m_print_mc_obs ) PrintModelObservables();

  if( m_print_nps ) PrintNuisanceParameters();

  if( m_print_nps_list ) PrintNuisanceParametersList();

  if( m_print_subchan )  PrintSubChannels();

  if(!kRun) return;

  //set all params to their initial values
  if( !LoadSnapshot(m_snapshot_initial_name) ) {cerr<<"NLLprofiler::Calculate - couldn't upload "<<m_snapshot_initial_name<<endl; exit(0); }

  //set first POI
  SetFirstPOI();

  //get roo sim pdf
  RooSimultaneous *simPdf = GetSimPDF();

  //check first POI
  if(m_fit_conditional) m_firstPOI->setConstant();

  //minimization
  RooMsgService::instance().setGlobalKillBelow(m_roofit_msg_lvl);//enum MsgLevel { DEBUG=0, INFO=1, PROGRESS=2, WARNING=3, ERROR=4, FATAL=5 } ; RooFit::WARNING

  ROOT::Math::MinimizerOptions::SetDefaultStrategy(m_strategy);
  int status_Minuit     = -1234;
  int status_HESSE      = -1234;
  double EDM                    = -1234;
  RooFitResult  *globalfit = m_fit_function->FitPDF( m_mc, simPdf, m_data, status_Minuit, status_HESSE, EDM, m_minimizer, m_minos, m_max_fit_retries, m_n_cpus );

  ///NLL scans
  cout<<"NLLprofiler::Calculate() - started scanning NPs"<<endl;

  const RooArgSet *ParamsGlobalFit = m_mc->GetNuisanceParameters();
  m_ws->saveSnapshot("snapshot_paramsVals_GlobalFit", *ParamsGlobalFit);

  double muhat = m_firstPOI->getVal();
  cout<<"NLLprofiler::Calculate() - muhat = "<<muhat<<endl;
  m_firstPOI->setConstant(false);


  //NLL scans start here
  RooAbsReal* nll = m_n_cpus ? simPdf->createNLL(*m_data, NumCPU(m_n_cpus) ) : simPdf->createNLL(*m_data);
  RooRealVar* var = NULL;
  bool floating(false);
  string ChannelName = "Combined";

  //scan POI
  if(m_scan_poi) if(!m_fit_conditional){
    //scan POI - Global
    var = (RooRealVar*) m_firstPOI;
    floating = true;
    Profile(nll, var, floating, ChannelName);

    if(m_fit_subchannels){
      //scan POI - per category
      RooCategory *cats = (RooCategory*) (&simPdf->indexCat());
      TIterator *itr_chan = cats->typeIterator() ;
      RooCatType *rct  = NULL;
      while((rct = (RooCatType*)itr_chan->Next()) ){
        string SubChannelName(rct->GetName());
        RooAbsPdf  *pdf_tmp  = simPdf->getPdf( rct->GetName() );
        RooAbsData *data_tmp = m_data->reduce( Form("%s==%s::%s", cats->GetName(), cats->GetName(), rct->GetName()) );
        RooAbsReal* cat_nll = m_n_cpus ? pdf_tmp->createNLL(*data_tmp, NumCPU(m_n_cpus)) : pdf_tmp->createNLL(*data_tmp);
        if(!cat_nll->dependsOn(*var)) continue;
        Profile(cat_nll, var, floating, SubChannelName);
      }
    }
  }

  //scan NPs
  TIterator* itr = m_nuisance->createIterator();
  while( (var = (RooRealVar*) itr->Next()) ){
    floating = false;
    TString vname=var->GetName();
    if (vname.Contains("gamma_stat")) continue;
    if(vname.Contains("ATLAS_norm")) floating = true;

    //scan a specific NP if asked
    if(m_scan_variable.size()) if( vname != m_scan_variable) continue;


    //scan NPs - Gombined
    Profile(nll, var, floating, ChannelName);

    if(m_fit_subchannels){
      //scan NPs - subchannesl
      RooCategory* cats = (RooCategory*) (&simPdf->indexCat());
      TIterator *itr_chan = cats->typeIterator() ;
      RooCatType *rct = NULL;
      while((rct = (RooCatType*)itr_chan->Next()) ){
        string SubChannelName(rct->GetName());
        //retrieve data and pdf to the current sub-channel
        RooAbsPdf  *pdf_tmp  = simPdf->getPdf( rct->GetName() );
        RooAbsData *data_tmp = m_data->reduce(Form("%s==%s::%s", cats->GetName(), cats->GetName(), rct->GetName()));
        RooAbsReal* cat_nll = m_n_cpus ? pdf_tmp->createNLL(*data_tmp, NumCPU(m_n_cpus)) : pdf_tmp->createNLL(*data_tmp);
        if(!cat_nll->dependsOn(*var)) continue;
        Profile(cat_nll, var, floating, SubChannelName);
      }
    }
  }


  cout<<"\nSuccess"<<endl;
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool NLLprofiler::SetProperties(){

  if(m_minos) m_n_cpus = 0;

  //output dir
  gSystem->Exec( ("mkdir -p " + GetOutputDirectoryName() ).c_str() );
  if( ! gSystem->OpenDirectory( GetOutputDirectoryName().c_str() ) ){ cerr<<"NLLprofiler::SetProperties - couldn't create directory "<<GetOutputDirectoryName()<<endl; return false; }

  //outputfile
  m_outfile = new TFile( (GetOutputDirectoryName() + m_outfile_name +".root").c_str(), "recreate");

  //input file
  m_ws_file = TFile::Open( (GetInputFileName()).c_str() );
  if (!m_ws_file) { cerr << "NLLprofiler::SetProperties - unable to open the file " << GetInputFileName() << endl; return false;        }
  Long_t id,size,flags,mt;
  gSystem->GetPathInfo( GetInputFileName().c_str(),&id,&size,&flags,&mt);
  if( size < 10000 ){    cerr << "NLLprofiler::SetProperties - trying to open a file with too few bytes" << GetInputFileName() << " " << size << endl; return false; }
  if( m_ws_file->TestBit(TFile::kRecovered) ){cerr << "NLLprofiler::SetProperties - trying to open an unclosed file " << GetInputFileName() << endl; return false;}
  if( m_ws_file->IsZombie() ){ cerr << "NLLprofiler::SetProperties - zombie file " << GetInputFileName() << endl; return false; }

  //workaspace
  m_ws      = (RooWorkspace*) m_ws_file->Get( GetWorkspaceName().c_str() );
  if(!m_ws) {cerr << "NLLprofiler::SetProperties - unable to obtain WS " << GetWorkspaceName() << "from file "<< GetInputFileName() <<endl; return false;}

  //model config
  m_mc = (ModelConfig*) m_ws->obj( GetModelConfigName().c_str() );
  if(!m_mc) {cerr << "NLLprofiler::SetProperties - unable to obtain MC " << GetModelConfigName() << "from WS "<< GetWorkspaceName() <<endl; m_ws->Print(); return false;}

  //data
  m_data = (RooAbsData*)m_ws->data( GetObsDataName().c_str() );
  if(!m_data) {cerr << "NLLprofiler::SetProperties - unable to retrieve data " << GetObsDataName() << "from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}

  //pdf
  string m_pdf_name = "simPdf";
  m_pdf = (RooSimultaneous*) m_ws->pdf(m_pdf_name.c_str());
  if(!m_pdf) {cerr << "NLLprofiler::SetProperties - unable to retrieve PDF " <<  m_pdf_name<< "from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}

  //parameters
  m_parameters = (RooArgSet*) m_pdf->getParameters(*m_data) ;
  if(!m_parameters) {cerr << "NLLprofiler::SetProperties - unable to load data parameters from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}

  //observables
  m_observables = (RooArgSet*) m_mc->GetObservables();
  if(!m_observables) {cerr << "NLLprofiler::SetProperties - unable to get all observables from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}

  //nuisance params
  m_nuisance = (RooArgSet*)m_mc->GetNuisanceParameters();
  if(!m_nuisance) {cerr << "NLLprofiler::SetProperties - unable to get NPs from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}

  //load initial snapshot
  if( !LoadSnapshot(m_snapshot_initial_name) ){
    if( m_ws->saveSnapshot(m_snapshot_initial_name.c_str(), *m_parameters) ){
      cout<<"NLLprofiler::SetProperties - snapshot "<< m_snapshot_initial_name <<" is created and stored"<<endl;
    }else{
      cerr<<"NLLprofiler::SetProperties - couldn't create and save snapshot '"<<m_snapshot_initial_name<<"'"<<endl; return false;
    }
  }else{
    cout<<"NLLprofiler::SetProperties - Snapshot "+m_snapshot_initial_name+" already exists in  workspace "+GetWorkspaceName()+" and it will not be overwritten" << endl;
  }

  //check sim pdf
  if(! CheckSimultaneousPDF() ) { cerr << "NLLprofiler::SetProperties - problems with simultaneous PDF " <<endl; m_ws->Print(); return false; }

  //check channel name
  if(! CheckChannelName() ){cerr << "NLLprofiler::SetProperties - problems with the channel name in "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}

  //NP map
  if( ! ContructNPmap() ){ cerr<<"NLLprofiler::SetProperties - couldn't create the NP map "<<endl; return false;}

  //the fitting PDF
  m_fit_function = new FitFunction(m_print_fit_pdf_model, m_print_fit_pdf_init_params, m_RooMinimizer_print_lvl);
  if(!m_fit_function){ cerr<<"NLLprofiler::SetProperties - couldn't create the fit PDF class object"<<endl; return false;}

  return true;
}
// Get the RooSimultaneous PDF
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void NLLprofiler::Profile(RooAbsReal *nll, RooRealVar *var, bool floating, const string &channelName){

  TString vname = var->GetName();
  
  vname.ReplaceAll("alpha_","");
  vname.ReplaceAll("gamma_","");
  vname.ReplaceAll("ATLAS_","");



  RooPlot *frame0 = var->frame( Title(vname+" : "+channelName));//, Range(m_nnl_range_low, m_nnl_range_high) );

  //red
  cout<<"NLLprofiler::Profile - scanning red NNL profile for "<<vname<<" in "<<channelName<<" ..."<<endl;
  nll->plotOn(frame0, LineColor(kRed), ShiftToZero(), Name("redCurve") );

  RooCurve* curve_0 = frame0->getCurve(); if(!curve_0){cerr<<"NLLprofiler::Profile - can't cut red curve. Exiting ... "<<endl; exit(0);}

  //blue - poisson term
  RooCurve* curve_1 = frame0->getCurve();

  double min(0);
  if(!floating) {
    cout<<"NLLprofiler::Profile - scanning blue NNL profile for "<<vname<<" in "<<channelName<<" ..."<<endl;
    RooCurve *poissonTerm = new RooCurve(*curve_1);
    poissonTerm->SetName("blueCurve");
    for(int i=1; i < curve_1->GetN()-2; i++) {
      poissonTerm->SetPoint(i, curve_1->GetX()[i], curve_1->GetY()[i] - curve_1->GetX()[i]*curve_1->GetX()[i]/2.);
      if(poissonTerm->GetY()[i] < min) { min = poissonTerm->GetY()[i]; }
    }
    poissonTerm->SetLineColor(kBlue);
    frame0->addPlotable(poissonTerm, "same", false, false);
    frame0->GetXaxis()->SetRangeUser(m_nnl_range_low, m_nnl_range_high);
    //    curve_1 =  poissonTerm;
    curve_1 = dynamic_cast<RooCurve*>(poissonTerm);
    //if(poissonTerm) delete poissonTerm;
  } else {
    float val = var->getVal();
    float minVal = 0;
    float maxVal = 2;
    if(val>1/5) {
      minVal = val - 2; if(minVal<0) { minVal = 0; }
      maxVal = val + 2;
    }
    frame0->GetXaxis()->SetRangeUser(minVal,maxVal);
  }

  //green
  RooCurve* curve_2 = 0;
  if(m_scan_green){
    cout<<"NLLprofiler::Profile - scanning green NNL profile for "<<vname<<" in "<<channelName<<" ..."<<endl;
    TStopwatch time;
    time.Start();
    RooAbsReal* pll = nll->createProfile( RooArgSet(*var) );
    if(!pll) {cerr<<"NLLprofiler::Profile - profile for the green nll of varible "<<vname<<" couldn't be created"<<endl; return;}
    pll->SetName("greenCurve");
    pll->plotOn(frame0, LineColor(kGreen), ShiftToZero(), Precision(0.05), Name("greenCurve") ) ;
    cout<<"\ttime = "; time.Print(); cout<<endl;
    curve_2 = (RooCurve*)frame0->findObject("greenCurve");
    time.Print();
    cout<<endl;
  }

  //frame
  frame0->updateYAxis(min, 12.0);
  frame0->GetYaxis()->SetRangeUser(min, 12.0);
  frame0->GetYaxis()->SetTitle("#Delta [-Log(L)]");
  frame0->GetXaxis()->SetRangeUser(m_nnl_range_low, m_nnl_range_high);

  //get mean and error strings
  string parabola_0 = GetParabolaMeanAndErrorsString(curve_0);
  string parabola_1 = GetParabolaMeanAndErrorsString(curve_1);
  string parabola_2 = m_scan_green ? GetParabolaMeanAndErrorsString(curve_2) : "";

  //canvas
  TString canvas_name = "c_" + channelName + "_" + vname;
  TCanvas canvas( canvas_name.Data(), "", 10, 10, 600, 600);
  canvas.cd();
  frame0->Draw();

  string latex_0 = "#color[2]{"+parabola_0+"}";
  string latex_1 = "#color[4]{"+parabola_1+"}";
  string latex_2 = "#color[3]{"+parabola_2+"}";
  string latex = latex_0 + "  " + latex_1 + ( m_scan_green ? "  " + latex_2 : "" );

  TLatex lat;
  lat.SetNDC();
  lat.DrawLatex(0.3, 0.85, latex.c_str());


  m_outfile->cd();
  canvas.Write();


}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
string NLLprofiler::GetParabolaMeanAndErrorsString(RooCurve *rc){

  CurveFitFunction *fitf = new CurveFitFunction(rc);
  TF1 f("f", fitf, -5, 5, 0, "FitFunction");
  double mean = f.GetMinimumX();
  double dy = 0.1;
  double xrange = 5;
  double left = f.GetX( dy , mean-xrange, mean);
  double right = f.GetX( dy , mean,mean + xrange);

  double errL = fabs(mean - left);
  double errH = fabs(mean - right);

  char mean_char[12];
  char errL_char[12];
  char errH_char[12];
  sprintf(mean_char,"%.2f", mean);
  sprintf(errL_char,"%.2f", errL);
  sprintf(errH_char,"%.2f", errH);

  delete fitf;

  return char2str(mean_char) + "^{+" + char2str(errH_char) + "}_{-" + char2str(errL_char) + "}";

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void NLLprofiler::SetFirstPOI(){

  m_firstPOI = dynamic_cast<RooRealVar*>(m_mc->GetParametersOfInterest()->first());
  m_firstPOI->setVal(m_poi);
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
RooCategory *NLLprofiler::GetCategories(){

  return (RooCategory*) & GetSimPDF()->indexCat();
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
RooSimultaneous *NLLprofiler::GetSimPDF(){

  return (RooSimultaneous*)m_mc->GetPdf();
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool NLLprofiler::LoadSnapshot(const string &snapshot){

  return m_ws->loadSnapshot( snapshot.c_str() );
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void NLLprofiler::PrintModelObservables(){

  TIterator* iter = m_observables->createIterator() ;
  RooAbsArg* arg = NULL;
  cout << "----------------------------"  << endl;
  cout << "List of Model Observables : "  << endl;
  cout << "----------------------------"  << endl;
  int i(0);
  while( (arg = (RooAbsArg*) iter->Next()) ){
    arg->Print(); i++;
  }
  cout<<"total : "<<i<<endl<<endl;

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void NLLprofiler::PrintNuisanceParameters(){

  TIterator* itr = m_nuisance->createIterator();
  RooRealVar* arg = NULL;
  cout << "----------------------------"  << endl;
  cout << "All Nuisance Parameters : "  << endl;
  cout << "----------------------------"  << endl;
  int i(0);
  while ((arg=(RooRealVar*)itr->Next())) {
    if (!arg) continue;
    i++;
    cout << arg->GetName()  << " : " << arg->getVal() << "+/-" << arg->getError()
         << " NormFactor = "<< (NormFactor(arg)?"yes":"no") << endl;
  }
  cout<<"total : "<<i<<endl<<endl;

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void NLLprofiler::PrintNuisanceParametersList(){

  TIterator* itr = m_nuisance->createIterator();
  RooRealVar* arg = NULL;
  cout << "----------------------------"  << endl;
  cout << "List ofNuisance Parameters : "<< endl;
  cout << "----------------------------"  << endl;

  cout<<"\nNPlist=("<<endl;
  while ((arg=(RooRealVar*)itr->Next())) {
    if (!arg) continue;
    string name = arg->GetName();
    if( StrHasSubstr(name, "alpha") || NormFactor(arg) )
      cout<<name<<endl;
  }
  cout<<")"<<endl;

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void NLLprofiler::PrintSubChannels(){

  RooMsgService::instance().setGlobalKillBelow(ERROR);

  //    RooSimultaneous* simPdf = (RooSimultaneous*)(m_mc->GetPdf());
  RooCategory* channelCat = GetCategories();
  //    (RooCategory*) (&simPdf->indexCat());
  TIterator* iter = channelCat->typeIterator() ;
  RooCatType* t = NULL;
  while((t=(RooCatType*) iter->Next()) ){

    RooAbsPdf  *pdftmp  = m_pdf->getPdf( t->GetName() );
    RooAbsData *datatmp = m_data->reduce(Form("%s==%s::%s", channelCat->GetName(), channelCat->GetName(), t->GetName()));

    cout << "----------------------------------------------------------" << endl;
    cout << "Channel details " << t->GetName() << " : "  << endl;
    cout << "----------------------------------------------------------" << endl;
    datatmp->Print();
    pdftmp->Print();
    PrintNumberOfEventsPDF(pdftmp);

  }

  RooMsgService::instance().setGlobalKillBelow(m_roofit_msg_lvl);

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void NLLprofiler::PrintNumberOfEventsPDF(RooAbsPdf *pdf){

  RooRealVar* firstPOI = (RooRealVar*) m_mc->GetParametersOfInterest()->first();
  double val_sym=1;
  cout
    << Form(" %3s |","")
    << Form(" %-32s |","Nuisance Parameter")
    << Form(" %18s |","Signal events")
    << Form(" %18s |","% Change (+1sig)")
    << Form(" %18s |","% Change (-1sig)")
    << Form(" %18s |","Background events")
    << Form(" %18s |","% Change (+1sig)")
    << Form(" %18s |","% Change (-1sig)")
    << endl;

  int inuis=-1;
  RooArgSet  *obstmp  = pdf->getObservables( *m_mc->GetObservables() ) ;
  RooRealVar *myobs   = ((RooRealVar*) obstmp->first());

  RooArgSet nuis = *m_mc->GetNuisanceParameters();
  TIterator* itr = nuis.createIterator();
  RooRealVar* arg;
  while ((arg=(RooRealVar*)itr->Next())) {
    if (!arg) continue;

    ++inuis;

    double val_hi = val_sym;
    double val_lo = -val_sym;
    double val_nom = arg->getVal();
    if (string(arg->GetName()) == "Lumi"){
      val_nom = m_ws->var("nominalLumi")->getVal();
      val_hi  = m_ws->var("nominalLumi")->getVal() * (1+m_LumiRelError);
      val_lo  = m_ws->var("nominalLumi")->getVal() * (1-m_LumiRelError);
    }
    //
    arg->setVal(val_hi);
    firstPOI->setVal(0);
    double b_hi = pdf->expectedEvents(*myobs);
    firstPOI->setVal(1);
    double s_hi = pdf->expectedEvents(*myobs)-b_hi;
    //
    arg->setVal(val_lo);
    firstPOI->setVal(0);
    double b_lo = pdf->expectedEvents(*myobs);
    firstPOI->setVal(1);
    double s_lo = pdf->expectedEvents(*myobs)-b_lo;
    //
    arg->setVal(val_nom);
    firstPOI->setVal(0);
    double b_nom = pdf->expectedEvents(*myobs);
    firstPOI->setVal(1);
    double s_nom = pdf->expectedEvents(*myobs)-b_nom;
    //
    double x_nom = s_nom ;
    double x_hi  = 0; if (s_nom) x_hi = (s_hi-s_nom)/s_nom;
    double x_lo  = 0; if (s_nom) x_lo = (s_lo-s_nom)/s_nom;
    double y_nom = b_nom ;
    double y_hi  = 0; if (b_nom) y_hi = (b_hi-b_nom)/b_nom;
    double y_lo  = 0; if (b_nom) y_lo = (b_lo-b_nom)/b_nom;

    cout
      << Form(" %3d |",inuis)
      << Form(" %-32s |",arg->GetName())
      << Form(" %18.2f |",x_nom)
      << Form(" %18.2f |",100*x_hi)
      << Form(" %18.2f |",100*x_lo)
      << Form(" %18.2f |",y_nom)
      << Form(" %18.2f |",100*y_hi)
      << Form(" %18.2f |",100*y_lo)
      << endl;
  }

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool NLLprofiler::ContructNPmap(){

  m_NP_map.clear();

  TIterator *it = m_mc->GetNuisanceParameters()->createIterator();
  RooRealVar *var = NULL;
  while ((var = (RooRealVar*)it->Next()) != NULL)
    m_NP_map[(string)var->GetName()] =  var->getVal();

  return !m_NP_map.empty();

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool NLLprofiler::NormFactor(RooRealVar *var){

  string VarName = (string) var->GetName();
  double VarVal =  m_NP_map[ VarName ];

  return ( !StrHasSubstr(VarName, "_stat_") && VarVal==1.0 );
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool NLLprofiler::StrHasSubstr(const string &str, const string &sub){

  std::size_t found = str.find(sub);

  return found!=std::string::npos;

}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool NLLprofiler::CheckSimultaneousPDF(){

  if (strcmp(m_mc->GetPdf()->ClassName(),"RooSimultaneous") ){
    cout << " ERROR : no Simultaneous PDF was found, must check all input histograms." << endl;
    return false;
  }

  return true;
}

///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool NLLprofiler::CheckChannelName(){

  //    RooSimultaneous* simPdf = (RooSimultaneous*)(mc->GetPdf());

  RooCategory* channelCat = (RooCategory*) (& m_pdf->indexCat() );
  TIterator* iter = channelCat->typeIterator() ;
  RooCatType* t = NULL;
  while( (t = (RooCatType*)iter->Next()) ){
    string channelName =  t->GetName();
    if ( StrHasSubstr(channelName, "/") ){
      cout << "Channel "+channelName+" contains a character \"/\" : This is mis-intrepreted by roofit in the reading of the workspace. Change the channel name in the xml file to run this code." << endl;
      return false;
    }
  }


  return true;
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
