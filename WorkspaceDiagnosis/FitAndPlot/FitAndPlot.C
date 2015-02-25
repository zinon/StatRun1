#include "../RooHeaders/RooHeaders.h"
#include "../FitFunction/FitFunction.h"

#include <map>
#include <sys/stat.h>

#include "TFile.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TString.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TStyle.h"


using namespace std;

#include <iostream>


void Run(const string &, const string &,const string &, const string &, const string &, bool, double, int);

void FitAndPlot();

void FitAndPlot(){

  string file = "../workspaces/current/output_unblinded_tes_mfs_sym/xml_measurement_hh_combination_125_results/hist2workspace_combined_measurement_hh_combination_125_model.root";
  bool kConditional = false;
  double mu =1.;
  int DefaultStrategy = 1;

  Run(file, "results", "combined", "ModelConfig", "obsData", kConditional, mu, DefaultStrategy);

}


void Run(const string &RooFileName, const string & OutDirName,const string &WorkspaceName, const string &ModelConfigName, const string &ObsDataName, bool kCond, double mu,
         int DefaultStrategy)
{
  //roofit verbosity
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);//enum MsgLevel { DEBUG=0, INFO=1, PROGRESS=2, WARNING=3, ERROR=4, FATAL=5 } ; RooFit::WARNING

  //output file
  TFile *fout = new TFile("fitresults.root", "recreate");

  //accellerate science
  SysInfo_t* s = new SysInfo_t(); gSystem->GetSysInfo(s); int Ncpus = s->fCpus; cout<<"CPUs "<<Ncpus<<endl;

  //initial snapshot
  string snapName = "snapshot_paramsVals_initial";

  //input roo file
  TFile *f = TFile::Open(RooFileName.c_str());
  if(!f) { cerr << "Cannot load file " << RooFileName <<endl; return;}

  //obtain roo-workspace
  RooWorkspace *w = (RooWorkspace*) f->Get(WorkspaceName.c_str());
  if(!w){ cerr<<"Cannot load workspace "<<WorkspaceName<<endl; return; }
  w->SetName("w"); w->SetTitle("w");

  ModelConfig  *mc = (ModelConfig*) w->obj(ModelConfigName.c_str());

  RooAbsData   *data = w->data(ObsDataName.c_str());

  //save snapshot before any fit
  RooSimultaneous *pdf = (RooSimultaneous*) w->pdf("simPdf");

  //get parameters
  RooArgSet* params = (RooArgSet*) pdf->getParameters(*data);

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
  RooFitResult  *fitresGlobal = fit_function->FitPDF( mc, simPdf, data, Minuit_Status, Hesse_Status, EDM, /*minimizer*/"Minuit2", /*minos*/false, /*max fit retries*/3, /*n cpus*/ 0);


  //TString FitName = "GlobalFit_" + TS_IsConditionnal + "_mu";
  //FitName += mu;
  //AllFitResults_map[FitName] = fitresGlobal;
  //AllFitStatus_map[FitName] = status;

  const RooArgSet *ParaGlobalFit = mc->GetNuisanceParameters();
  w->saveSnapshot("snapshot_paramsVals_GlobalFit", *ParaGlobalFit);

  double muhat = firstPOI->getVal();
  firstPOI->setConstant(kFALSE);

  if (kCond)
    cout << "Conditionnal fit : mu is fixed at " << mu << endl;
  else
    cout << "Unconditionnal fit : mu is fitted" << endl;

  //fitresGlobal->Print("v");

  // Plotting the distributions for each subchannel
  RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
  TIterator *iter = channelCat->typeIterator() ;
  RooCatType *tt  = NULL;
  while((tt=(RooCatType*) iter->Next()) ){
    string channel_str(tt->GetName());

    cout<<"\n==>Channel "<<channel_str<<endl;
    if( ((string)channel_str).find("vbf") == string::npos ) continue;

    RooAbsPdf  *pdftmp  = simPdf->getPdf( tt->GetName() );
    RooAbsData *datatmp = data->reduce(Form("%s==%s::%s",channelCat->GetName(),channelCat->GetName(),tt->GetName()));
    RooArgSet  *obstmp  = pdftmp->getObservables( *mc->GetObservables() ) ;
    RooRealVar *obs     = ((RooRealVar*) obstmp->first());

    // Bin Width
    RooRealVar* binWidth = ((RooRealVar*) pdftmp->getVariables()->find(Form("binWidth_obs_x_%s_0",tt->GetName()))) ;
    if(!binWidth) { cout << "No bin width " << tt->GetName() << endl; }
    cout << "\n==> Bin Width : " << binWidth->getVal() << endl;

    // Load the value from the global fit
    if(!w->loadSnapshot(snapName.c_str())) { cerr << "Cannot load " << snapName << endl;}

    if(!kCond){
      firstPOI->setVal(muhat);
      cout<<"\n==>unconditional fit --> setting POI to "<<muhat<<" in channel "<<channel_str<<endl;
    }else{
      firstPOI->setVal(mu);
      cout<<"\n==>conditional fit --> setting POI to "<<mu<<" in channel "<<channel_str<<endl;
    }
    string modelName( tt->GetName() );
    RooRealSumPdf *pdfmodel = (RooRealSumPdf*) (pdftmp->getComponents())->find((modelName+"_model").c_str());
    if(!pdfmodel){cerr<<"cannot create pdf model in channel "<<channel_str<<endl; return;}

    RooArgList funcList =  pdfmodel->funcList();
    if(!funcList.getSize()){cerr<<"func list is empty in channel "<<channel_str<<endl; return;}
    RooProduct* comp = 0;
    RooLinkedListIter funcIter = funcList.iterator() ;
    cout << "\n==>Post Fit " << endl;
    while( (comp = (RooProduct*) funcIter.Next()) ) {
      cout << "\t" << comp->GetName() << "\t" << (comp->createIntegral(*obs))->getVal() * binWidth->getVal() << endl;
    }

    //canvas,
    TCanvas *c = new TCanvas( "c", channel_str.c_str(),10,10, 800, 500);
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    c->SetFillColor(0);
    c->SetBorderMode(0);
    c->SetBorderSize(0);
    c->SetTickx(1);
    c->SetTicky(1);
    c->SetRightMargin(0.025);
    c->SetTopMargin(0.08);
    c->SetFrameBorderMode(0);
    c->SetFrameBorderMode(0);

    RooPlot *frame = obs->frame();
    frame->SetName( channel_str.c_str() );
    frame->SetYTitle("Events");
    frame->SetXTitle("BDT bin");

    //create a roo-plot
    RooPlot *xframe = obs->frame();

    float postFitIntegral = pdftmp->expectedEvents(*obs);

    //important to calculate the chi2
    datatmp->plotOn(frame, MarkerSize(1), Name("Visible_Data"), DataError(RooAbsData::Poisson));

    pdftmp->plotOn(frame, 
		   FillColor(kOrange), 
		   FillStyle(3004), 
		   LineWidth(0), 
		   LineColor(0), 
		   LineStyle(7),
                   VisualizeError(*fitresGlobal,1),
                   Normalization(postFitIntegral,RooAbsReal::NumEvent),
		   Name("FitError_NotAppears"));
    frame->setInvisible("FitError_NotAppears", kTRUE);

    pdftmp->plotOn(frame,
		   LineWidth(0), 
		   LineColor(0), 
		   LineStyle(7), 
		   //DrawOption("f"),
		   //FillColor(kGray),  
		   //Range(0., 101.)
                   Normalization(postFitIntegral,RooAbsReal::NumEvent),  
		   Name("CentralFit_NotAppears"));
    frame->setInvisible("CentralFit_NotAppears", kTRUE);

    double chi2 = frame->chiSquare();

    cout<<"==> chi2 "<<chi2<<endl;

    //loop to create stack plots
    string modelName_mini(tt->GetName());
    modelName_mini+="_model";
    RooRealSumPdf *pdfmodel_mini = (RooRealSumPdf*) (pdftmp->getComponents())->find(modelName_mini.c_str());
    RooArgList funcList_mini =  pdfmodel_mini->funcList();
    RooLinkedListIter funcIter_mini = funcList_mini.iterator();
    RooProduct* comp_mini = 0;
    if (!kCond){
      firstPOI->setVal(muhat);
      cout<<"\n==> unconditional fit, POI set to "<<muhat<<endl;
    }else{
      firstPOI->setVal(mu);
      cout<<"\n==> conditional fit, POI set to "<<mu<<endl;
    }

    //count bkg components
    int nBkgComp(0);
    while( (comp_mini = (RooProduct*) funcIter_mini.Next()) ) {
      TString compname(comp_mini->GetName());
      if(!compname.Contains("Signal")) nBkgComp++;
    }
    cout<<"\n==> N bkgs "<<nBkgComp<<endl;

    TString previous("");
    TString previous_bkg("");
    int iBkg(0);
    int iBkgSig(0);
    int nBkgSigCom = funcList_mini.getSize();
    cout<<"\n==> loop over "<< nBkgSigCom <<" components"<<endl;
    funcIter_mini = funcList_mini.iterator();
    while( (comp_mini = (RooProduct*) funcIter_mini.Next()) ) {
      TString compname(comp_mini->GetName());
      compname.ReplaceAll("L_x_","");
      compname.ReplaceAll(tt->GetName(),"");
      compname.ReplaceAll("_overallSyst_x_StatUncert","");
      compname.ReplaceAll("_overallSyst_x_HistSyst","");
      compname.ReplaceAll("_overallSyst_x_Exp","");
      compname.ReplaceAll("_","");

      // Get the signal x 1 in white
      //if(kCond) firstPOI->setVal(1.0);
      //double Ntemp=(comp_mini->createIntegral(*obs))->getVal() * binWidth->getVal();
      //pdfmodel_mini->plotOn(frame, LineWidth(0), Components(*comp_mini), LineColor(0), LineStyle(3),
      //                      Normalization(Ntemp,RooAbsReal::NumEvent), Name("NoStacked_"+compname) );
      //if(kCond) firstPOI->setVal(mu);

      // Stack bkg + signal
      iBkgSig++;
      int Ntemp = (comp_mini->createIntegral(*obs))->getVal() * binWidth->getVal();
      if(iBkgSig == 1 ){
        pdfmodel_mini->plotOn(frame, LineWidth(0), Components(*comp_mini), LineColor(0), Normalization(Ntemp,RooAbsReal::NumEvent), Name("Stacked_"+compname));
        frame->setInvisible("Stacked_"+compname, kTRUE);
        cout<<"\n==> First Component Bkg or Sig "<<iBkgSig<<"/"<<nBkgSigCom <<" : "<<compname<<endl;
      }else if( iBkgSig < nBkgSigCom){
        pdfmodel_mini->plotOn(frame, LineWidth(0), Components(*comp_mini), LineColor(0), Normalization(Ntemp,RooAbsReal::NumEvent), Name("Stacked_"+compname), AddTo(previous));
        frame->setInvisible("Stacked_"+compname, kTRUE);
        cout<<"\n==> Stack Component Bkg or Sig "<<iBkgSig<<"/"<<nBkgSigCom <<" : "<<compname<<endl;
      }else{
        pdfmodel_mini->plotOn(frame, Components(*comp_mini), 
			      LineWidth(2), 
			      LineColor(kMagenta), 
			      //FillColor(kMagenta-6),
                              DrawOption("l"),
                              //Range(0., 1e3), //rootfit has a bug in filled plots!
                              Normalization(Ntemp,RooAbsReal::NumEvent), 
			      Name("Visible_Stacked_"+compname), 
			      AddTo(previous));
        cout<<"\n==> Add last Component Bkg or Sig "<<iBkgSig<<"/"<<nBkgSigCom <<" : "<<compname<<endl;
      }
      previous="Stacked_"+compname;


      //Stack Bkg only
      if(!compname.Contains("Signal") ){
        iBkg++;

        if(iBkg == 1){
          pdfmodel_mini->plotOn(frame, Components(*comp_mini), LineWidth(0), LineColor(0),  /*LineStyle(7),*/
                                Normalization(Ntemp,RooAbsReal::NumEvent), Name("StackedBkg_"+compname));
          frame->setInvisible("StackedBkg_"+compname, kTRUE);
          cout<<"\n==> First Component Bkg "<<iBkg<<"/"<<nBkgComp<<" : "<<compname<<endl;
        }else if(iBkg < nBkgComp){
          pdfmodel_mini->plotOn(frame, Components(*comp_mini), LineWidth(0), LineColor(0), /*LineStyle(1),*/
                                Normalization(Ntemp,RooAbsReal::NumEvent), Name("StackedBkg_"+compname), AddTo(previous_bkg));
          cout<<"\n==> Add further component Bkg "<<iBkg<<"/"<<nBkgComp<<" : "<<compname<<endl;
          frame->setInvisible("StackedBkg_"+compname, kTRUE);
        }else{
          pdfmodel_mini->plotOn(frame, Components(*comp_mini), 
				LineWidth(2), 
				LineColor(kGreen+2), 
				LineStyle(1), 
				DrawOption("l"), 
				//FillColor(kGreen-6), 
				//Range(0., 101),
                                Normalization(Ntemp,RooAbsReal::NumEvent), 
				Name("Visible_StackedBkg_"+compname), 
				AddTo(previous_bkg));
          cout<<"\n==> Last component Bkg "<<iBkg<<"/"<<nBkgComp<<" : "<<compname<<endl;
        }
        previous_bkg="StackedBkg_"+compname;

      }


    }//loop over components

    //2258
    if(!kCond)
      firstPOI->setVal(muhat);
    else
      firstPOI->setVal(mu);

    pdftmp->plotOn(frame, FillColor(kOrange), FillStyle(3004), LineWidth(2), LineStyle(7), LineColor(kBlue),
                   VisualizeError(*fitresGlobal,1), Normalization(postFitIntegral,RooAbsReal::NumEvent), Name("Visible_FitError_AfterFit"));


    //reenable the error bands
    pdftmp->plotOn(frame, LineWidth(2), LineColor(kBlue), LineStyle(7),
                   Normalization(postFitIntegral, RooAbsReal::NumEvent), Name("FitCentral2_NotAppears"));

    //force data points to come on top
    datatmp->plotOn(frame, MarkerSize(1), Name("Data_NotAppears"), DataError(RooAbsData::Poisson));


    cout<<"\n==>List of object frames in frame"<<endl;
    frame->Print("v");
    //xframe->Print("v");

    //draw
    c->cd();
    TString cname = channel_str; 
    cname.ReplaceAll("_", " ");
    frame->SetTitle(cname);
    frame->SetMinimum(0.9);
    frame->SetMaximum( frame->GetMaximum() * 1.10 );
    frame->Draw();
    gPad->SetLogy();
    frame->SetTitle(channel_str.c_str());
    //xframe->Draw("same");
    gPad->RedrawAxis();

    TLatex text;
    text.SetNDC();
    text.SetTextSize( 0.054);
    text.SetTextAlign(31);
    TString MuValueStr;
    if(!kCond) MuValueStr = "#mu_{best} = ";
    else MuValueStr = "#mu_{fixed} = ";
    MuValueStr += Form("%2.2f",firstPOI->getVal());
    text.DrawLatex( 0.73,0.81, MuValueStr );
    TString ts_chi2 = Form("#chi^{2}=%1.1f", chi2 );
    text.DrawLatex( 0.22, 0.83, ts_chi2 );

    //legend
    TLegend *leg = new TLegend(0.54,0.60,0.7,0.76);
    leg->SetBorderSize(0);
    leg->SetFillColor(0);
    leg->SetTextFont(62);
    leg->SetTextSize(0.050);
    for (int i=0; i<frame->numItems(); i++) {

      TString obj_name=frame->nameOf(i);
      //if (obj_name=="" || obj_name.Contains("NotAppears") || obj_name.Contains("NoStacked") ) continue;
      if (!obj_name.Contains("Visible") ) continue;

      TObject *obj = frame->findObject(obj_name.Data());
      if (((string)obj_name).find("Data")   !=string::npos) {
        leg->AddEntry( obj , "Data" , "p");
        continue;
      }

      TString legname;
      if (!kCond)
        legname = "Post-fit (#mu[best] S+B)";
      else
        legname = "Post-fit (#mu[fixed] S+B)";

      if (((string)obj_name).find("AfterFit")!=string::npos){ leg->AddEntry( obj , legname , "lf"); continue; }

      //legname = muValueBeforeFitLegend;
      //if (((string)obj_name).find("BeforeFit")!=string::npos){leg->AddEntry( obj ,legname , "l"); continue;}

      obj_name.ReplaceAll("Visible_","");
      obj_name.ReplaceAll("Stacked_","");
      obj_name.ReplaceAll("StackedBkg_","");
      obj_name.ReplaceAll("Ztautau","B");
      obj_name.ReplaceAll("VBF","");
      obj_name.ReplaceAll("125","");
      obj_name.ReplaceAll("Signal","S+B");
      leg->AddEntry(obj, obj_name, "l");
    }

    leg->Draw();

    //save
    fout->cd();
    
    TString c_name("c_"+channel_str);
    c_name.ReplaceAll(" ", "_");
    
    c->Write( c_name );

  }//loop over channels/categories


  fout->Close();

}
