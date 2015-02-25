TLegend *GetLegend(double x1, double y1, double x2, double y2, TGraph *g, string cat, string para );

double GetMinNLL(TGraph *g);

void OverlayProfiles(){

  vector<string> v = GetNPlist();


  bool kScans = true;
  string WSname = "Run1PaperHadhad_MVA_r160507";
  string tag = "Apr28";
  string pref = kScans ? "NLLscans" : "NLLdep";
  string task = pref + "." + WSname + "." + tag;

  string NPref = "mu";

  system( ("mkdir -p eps/"+task).c_str() );
  system( ("mkdir -p png/"+task).c_str() );
  TFile *f = new TFile( ("plots/"+task+".root").c_str() );
  if(!f){cerr<<"can't open root file...\n"; return;}

  for(vector<string>::iterator u = v.begin(); u != v.end(); u++)
    Loop( *u , task, f, kScans, NPref);

}

void Loop(const string &np, string task, TFile *f, bool kScans, string NPref){

  string NP = np;

  bool kPng = true;
  bool kEps = true;

  string ov0 = "combination"; bool k0(true);
  string ov1 = "combination11"; bool k1(false);
  string ov2 = "combination12"; bool k2(false);
  string ov3 = ""; bool k3(false);

  string pref = kScans ? "nll_prof" : "dep";

  NP = kScans ? NP : NP + "_vs_" + NPref;

  cout<<pref+"_combination_"+NP<<endl;

  TGraph *g0 = (TGraph*)f->Get( (pref+"_"+ov0+"_"+NP).c_str()); if(!g0){ cout<<"can't get g0"<<endl; k0 = false; }
  TGraph *g1 = (TGraph*)f->Get( (pref+"_"+ov1+"_"+NP).c_str()); if(!g1){ cout<<"can't get g1"<<endl; k1 = false; }
  TGraph *g2 = (TGraph*)f->Get( (pref+"_"+ov2+"_"+NP).c_str()); if(!g2){ cout<<"can't get g2"<<endl; k2 = false; }
  TGraph *g3 = (TGraph*)f->Get( (pref+"_"+ov3+"_"+NP).c_str()); if(!g3){ cout<<"can't get g3"<<endl; k3 = false; }

  cout<<ov0<<" "<<(k0?"yes":"no")<<" : "<<ov1<<" "<<(k1?"yes":"no")<<" : "<<ov2<<" "<<(k2?"yes":"no")<<" : "<<ov3<<" "<<(k3?"yes":"no")<<endl;

  if(!k0) return;

  TCanvas *ca1 = new TCanvas("ca1", NP.c_str(),303,85,600,600);
  ca1->cd();
  ca1->SetFillColor(0);
  ca1->SetBorderMode(0);
  ca1->SetBorderSize(0);
  //   ca1->SetGridx();
  //   ca1->SetGridy();
  ca1->SetRightMargin(0.015);
  ca1->SetTopMargin(0.025);
  ca1->SetFrameBorderMode(0);

  gPad->SetTicks(1,1);

  const char *c0 = g0->GetTitle();
  const char *c1;
  const char *c2;
  const char *c3;

  string s0(c0);
  string s1,s2,s3;
  string S;
  g0->Draw("apl"); if(kScans){ g0->SetMaximum(10); g0->SetMinimum(-1); }

  double xmin = g0->GetXaxis()->GetXmin();
  double xmax = g0->GetXaxis()->GetXmax();


  TLine *l1 = new TLine(xmin,1,xmax,1);
  l1->SetLineColor(kGray);
  l1->SetLineStyle(7);
  l1->Draw();

  TLatex *t1 = new TLatex(xmin - xmin*0.1, 1, "1#sigma");
  t1->SetTextFont(42);
  t1->Draw();

  TLine *l2 = new TLine(xmin,4,xmax,4);
  l2->SetLineColor(kGray);
  l2->SetLineStyle(7);
  l2->Draw();

  TLatex *t2 = new TLatex(xmin - xmin*0.1, 4, "2#sigma");
  t2->SetTextFont(42);
  t2->Draw();

  TLine *l0 = new TLine(xmin,0,xmax,0);
  l0->SetLineColor(kGray);
  l0->SetLineStyle(7);
  if(kScans) l0->Draw();

  double ymin0 = GetMinNLL(g0);

  TLine *lm = new TLine(ymin0,-1, ymin0,10);
  lm->SetLineColor(kGray);
  lm->SetLineStyle(7);
  if(kScans) lm->Draw();

  if(k3) {
    g3->Draw("l");
    c3 = g3->GetTitle();
    s3 =c3;
    S += "  "+s3;
  }
  if(k2){
    g2->Draw("l");
    c2 = g2->GetTitle();
    s2 = c2;
    S += "  "+s2;
  }

  if(k1){
    g1->Draw("l");
    c1 = g1->GetTitle();
    s1 = c1;
    S += "  "+s1;
  }

  //g0->SetTitle(s0.c_str());

  g0->GetXaxis()->SetRangeUser(-3,3);
  g0->SetTitle();
  g0->Draw("pl");

  double yd = 0.1;
  double yu = 0.2;
  double x0 = 0.12;
  double dx=0.15;
  double e = 0.15;

  TLegend *leg1;
  TLegend *leg2;
  TLegend *leg3;
  TLegend *leg0 =               GetLegend(x0,           yd, x0+dx,              yu, g0, ov0.c_str(), s0);
  double x1 = x0+dx+e;
  leg1 = k1?            GetLegend(x1,   yd, x1+dx,      yu, g1, ov1.c_str(), s1) : 0;
  double x2 = x1+dx+e-0.05;
  leg2 = k2 ?   GetLegend(x2,   yd, x2+dx,      yu, g2, ov2.c_str(), s2) : 0;
  double x3 = x2+dx+e;
  leg3 = k3 ?   GetLegend(x3,   yd, x3+dx,      yu, g3, ov3.c_str(), s3) : 0;

  leg0->Draw();

  if(k1) leg1->Draw();
  if(k2) leg2->Draw();
  if(k3) leg3->Draw();
  //    system( ("mkdir -p png/"+task) );


  if(kPng) ca1->SaveAs( ("png/"+task+"/"+pref+"_"+NP+".png").c_str() );
  if(kEps) ca1->SaveAs( ("eps/"+task+"/"+pref+"_"+NP+".eps").c_str() );

}

double GetMinNLL(TGraph *g){

  int n = g->GetN();
  double minY(10000); double minX(0);
  for   (int i = 1; i <= n; i++){
    double x,y;
    g->GetPoint(i,x,y);

    if(y<minY){
      minY = y;
      minX = x;
    }

  }

  return minX;
}

TLegend *GetLegend(double x1, double y1, double x2, double y2, TGraph *g, string cat, string para ){

  TLegend  *leg =       new TLegend(x1,y1,x2,y2,NULL,"brNDC");
  leg->SetBorderSize(0);
  leg->SetLineColor(0);
  leg->SetLineStyle(0);
  leg->SetLineWidth(0);
  leg->SetFillColor(0);
  leg->SetFillStyle(0);
  leg->AddEntry(g, (cat + "  " +  para ).c_str(),       "lp");
  leg->SetTextSize(0.0275);
  leg->SetTextFont(42);
  leg->SetTextAlign(12);

  return leg;
}


vector<string> GetNPlist(){

  // less nplist_WS203_combined.txt | sort | sed "s/$/\");/g" | sed "s/^/v.push_back(\"/"
  //ls NLLscanner/results/output_unblinded_random_mu_combination_1nd/| sort | sed "s/ScanAndFit_//g" |  sed "s/_fixed2.5_2.5.root//g" | sed "s/$/\");/g" | sed "s/^/v.push_back(\"/"
  //less ../NuisParamRepo.sh | sed "s/$/\");/g" | sed "s/^/v.push_back(\"/" | tr -d ' '

  vector<string> v;
  //v.push_back("SigXsecOverSM");

  v.push_back("ATLAS_norm_HH_2011_QCD");
  v.push_back("ATLAS_norm_HH_2011_Ztt");
  v.push_back("ATLAS_norm_HH_2012_QCD");
  v.push_back("ATLAS_norm_HH_2012_Ztt");
  v.push_back("alpha_ATLAS_ANA_EMB_ISOL");
  v.push_back("alpha_ATLAS_ANA_EMB_MFS_2011");
  v.push_back("alpha_ATLAS_ANA_EMB_MFS_2012");
  v.push_back("alpha_ATLAS_ANA_HH_2011_QCD");
  v.push_back("alpha_ATLAS_ANA_HH_2012_QCD");
  v.push_back("alpha_ATLAS_BR_tautau");
  v.push_back("alpha_ATLAS_JER_2011");
  v.push_back("alpha_ATLAS_JER_2012");
  v.push_back("alpha_ATLAS_JES_2011_Detector1");
  v.push_back("alpha_ATLAS_JES_2011_Eta_StatMethod");
  v.push_back("alpha_ATLAS_JES_2011_Modelling1");
  v.push_back("alpha_ATLAS_JES_2011_PileRho_TAU_GG");
  v.push_back("alpha_ATLAS_JES_2011_PileRho_TAU_QQ");
  v.push_back("alpha_ATLAS_JES_2012_Detector1");
  v.push_back("alpha_ATLAS_JES_2012_Eta_StatMethod");
  v.push_back("alpha_ATLAS_JES_2012_Modelling1");
  v.push_back("alpha_ATLAS_JES_2012_PileRho_TAU_GG");
  v.push_back("alpha_ATLAS_JES_2012_PileRho_TAU_QG");
  v.push_back("alpha_ATLAS_JES_2012_PileRho_TAU_QQ");
  v.push_back("alpha_ATLAS_JES_Eta_Modelling");
  v.push_back("alpha_ATLAS_JES_FlavComp_TAU_G");
  v.push_back("alpha_ATLAS_JES_FlavComp_TAU_Q");
  v.push_back("alpha_ATLAS_JES_FlavResp");
  v.push_back("alpha_ATLAS_LUMI_2011");
  v.push_back("alpha_ATLAS_LUMI_2012");
  v.push_back("alpha_ATLAS_MET_RESOSOFT_2011");
  v.push_back("alpha_ATLAS_MET_RESOSOFT_2012");
  v.push_back("alpha_ATLAS_MET_SCALESOFT_2011");
  v.push_back("alpha_ATLAS_MET_SCALESOFT_2012");
  v.push_back("alpha_ATLAS_TAU_ID_2011");
  v.push_back("alpha_ATLAS_TAU_ID_2012");
  v.push_back("alpha_ATLAS_TAU_JFAKE_2011");
  v.push_back("alpha_ATLAS_TAU_JFAKE_2012");
  v.push_back("alpha_ATLAS_TES_FAKE_FINAL_2011");
  v.push_back("alpha_ATLAS_TES_FAKE_TOTAL_2012");
  v.push_back("alpha_ATLAS_TES_TRUE_FINAL_2011");
  v.push_back("alpha_ATLAS_TES_TRUE_INSITUINTERPOL_2012");
  v.push_back("alpha_ATLAS_TES_TRUE_MODELING_2012");
  v.push_back("alpha_ATLAS_TES_TRUE_SINGLEPARTICLEINTERPOL_2012");
  v.push_back("alpha_ATLAS_TRIGGER_EMB_HH_2011");
  v.push_back("alpha_ATLAS_TRIGGER_EMB_HH_2012");
  v.push_back("alpha_ATLAS_TRIGGER_HH_2011");
  v.push_back("alpha_ATLAS_TRIGGER_HH_2012");
  v.push_back("alpha_ATLAS_UE_gg");
  v.push_back("alpha_ATLAS_UE_qq");
  v.push_back("alpha_Gen_Qmass_ggH");
  v.push_back("alpha_QCDscale_V");
  v.push_back("alpha_QCDscale_VH");
  v.push_back("alpha_QCDscale_ggH1in");
  v.push_back("alpha_QCDscale_ggH2in");
  v.push_back("alpha_QCDscale_qqH");
  v.push_back("alpha_pdf_Higgs_gg");
  v.push_back("alpha_pdf_Higgs_qq");
  v.push_back("alpha_pdf_qq");

  return v;
}
