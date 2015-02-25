#include<iostream>
#include<vector>
#include<iterator>
#include<string>

#include "TFile.h"
#include "TCanvas.h"

using namespace std;

bool Has(const string &str, const string &sub);
vector<string> GetFileList();
std::string strreplace(std::string &s,
			                      std::string toReplace,
		       std::string replaceWith);

void PlotNLL(){

  string WSName = "Run1PaperHadhad_MVA_r158198_8nh";
  string dir  = "results/"+WSName+"/";
  string out = "plots/"+WSName+"/";

  cout<<"output directory: "<<out<<endl;

  vector<string> vec = GetFileList();

  for(vector<string>::iterator it = vec.begin(); it != vec.end();it++){

    string toOpen = dir + *it;
    TFile *f = TFile::Open(toOpen.c_str());

    if(!f) {cerr<<"unable to open file "<<toOpen<<endl; return;}

    string s1;
    if( Has(toOpen, "nll_profiles_alpha_") ) s1 = "nll_profiles_alpha_"; 
    else if( Has(toOpen, "nll_profiles_") ) s1 = "nll_profiles_"; 
    else{}
    string s2 = ".root";
    string  NP = *it; 
    NP.erase(0, NP.find(s1) + s1.size() ); 
    NP.erase(NP.find(s2));
    if( Has(NP, "ATLAS_") ) strreplace(NP, "ATLAS_", "");

    cout<<"NP "<<NP<<" file "<<toOpen<<endl;

    TCanvas *c1 = (TCanvas *)f->Get( ("c_Combined_"+NP).c_str() );
    if(!c1) {cerr<<"unable to get  combined "<<NP<<endl; return;}

    /*
    TCanvas *c2 = (TCanvas *)f->Get( ("c_channel_boosted_125_11_"+NP).c_str() );
    if(!c2) {cerr<<"unable to get  "<< c2->GetName()<<endl; return;}

    TCanvas *c3 = (TCanvas *)f->Get( ("c_channel_vbf_125_11_"+NP).c_str() );
    if(!c3) {cerr<<"unable to get  "<< c3->GetName()<<endl; return;}

    TCanvas *c4 = (TCanvas *)f->Get( ("c_channel_boosted_125_12"+NP).c_str() );
    if(!c4) {cerr<<"unable to get  "<< c4->GetName()<<endl; return;}

    TCanvas *c5 = (TCanvas *)f->Get( ("c_channel_vbf_125_12"+NP).c_str() );
    if(!c5) {cerr<<"unable to get  "<< c5->GetName()<<endl; return;}
    */
    c1->SaveAs( (out+"Combined_"+NP+".png").c_str() );
    /*c2->SaveAs( (out+"channel_boosted_125_11_"+NP+".png").c_str() );
    c3->SaveAs( (out+"channel_vbf_125_11_"+NP+".png").c_str() );
    c4->SaveAs( (out+"channel_boosted_125_12"+NP+".png").c_str() );
    c5->SaveAs( (out+"channel_vbf_125_12"+NP+".png").c_str() );
    */
  }

}


vector<string> GetFileList(){

  //ls results/Run1PaperHadhad_MVA_r158198_8nh/ | sort | sed "s/$/\");/g" | sed "s/^/v.push_back(\"/"

  vector<string> v;


  v.push_back("nll_profiles_alpha_ATLAS_ANA_EMB_ISOL.root");
  v.push_back("nll_profiles_alpha_ATLAS_ANA_EMB_MFS.root");
  v.push_back("nll_profiles_alpha_ATLAS_ANA_HH_2011_QCD.root");
  v.push_back("nll_profiles_alpha_ATLAS_ANA_HH_2012_QCD.root");
  v.push_back("nll_profiles_alpha_ATLAS_BR_tautau.root");
  v.push_back("nll_profiles_alpha_ATLAS_FAKERATE_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_ISOL_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JER_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JER_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_2012_Detector1.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_2012_Eta_StatMethod.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_2012_Modelling1.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_2012_PileRho_TAU_GG.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_2012_PileRho_TAU_QQ.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_Detector_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_EtaMethod_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_EtaModelling_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_Eta_Modelling.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_FlavComp_TAU_G_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_FlavComp_TAU_G.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_FlavComp_TAU_Q_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_FlavComp_TAU_Q.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_FlavResp_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_FlavResp.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_Modelling_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_PURho_TAU_GG_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_JES_PURho_TAU_QQ_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_LUMI_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_LUMI_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_MET_RESOSOFT.root");
  v.push_back("nll_profiles_alpha_ATLAS_MET_RESOSOFTTERMS_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_MET_SCALESOFT.root");
  v.push_back("nll_profiles_alpha_ATLAS_MET_SCALESOFTTERMS_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_MFS_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_TAUID_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_TAU_ID_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_TAU_JFAKE_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_TES_FAKE_FINAL_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_TES_FAKE_TOTAL_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_TES_TRUE_FINAL_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_TES_TRUE_INSITUINTERPOL_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_TES_TRUE_MODELING_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_TES_TRUE_SINGLEPARTICLEINTERPOL_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_TRIGGER_EMB_HH_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_TRIGGER_EMB_HH_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_TRIGGER_HH_2011.root");
  v.push_back("nll_profiles_alpha_ATLAS_TRIGGER_HH_2012.root");
  v.push_back("nll_profiles_alpha_ATLAS_UE_gg.root");
  v.push_back("nll_profiles_alpha_ATLAS_UE_qq.root");
  v.push_back("nll_profiles_alpha_Gen_Qmass_ggH.root");
  v.push_back("nll_profiles_alpha_pdf_Higgs_gg.root");
  v.push_back("nll_profiles_alpha_pdf_Higgs_qq.root");
  v.push_back("nll_profiles_alpha_pdf_qq.root");
  v.push_back("nll_profiles_alpha_QCDscale_ggH1in.root");
  v.push_back("nll_profiles_alpha_QCDscale_ggH2in.root");
  v.push_back("nll_profiles_alpha_QCDscale_qqH.root");
  v.push_back("nll_profiles_alpha_QCDscale_VH.root");
  v.push_back("nll_profiles_alpha_QCDscale_V.root");
  v.push_back("nll_profiles_ATLAS_norm_HH_2011_QCD.root");
  v.push_back("nll_profiles_ATLAS_norm_HH_2011_Ztt.root");
  v.push_back("nll_profiles_ATLAS_norm_HH_2012_QCD.root");
  v.push_back("nll_profiles_ATLAS_norm_HH_2012_Ztt.root");




  return v;
}

std::string strreplace(std::string &s,
                      std::string toReplace,
                      std::string replaceWith)
{
  return(s.replace(s.find(toReplace), toReplace.length(), replaceWith));
}
bool Has(const string &str, const string &sub){

  std::size_t found = str.find(sub);

  if (found!=std::string::npos) return true;

  return false;
}
