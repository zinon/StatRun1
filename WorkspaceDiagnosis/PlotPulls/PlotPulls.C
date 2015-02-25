#include<vector>
#include<iostream>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

#include "Riostream.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TAxis.h"
#include "TBox.h"
#include "TLatex.h"
#include "TColor.h"
#include "TLine.h"
#include "TLegend.h"
#include "TPad.h"
#include "TEllipse.h"
#include "TGaxis.h"
#include "TH1D.h"
#include "TStyle.h"

using namespace std;

struct pull_t {

  std::string name;
  std::string category;
  double value;
  double errorUp;
  double errorDown;

};

typedef vector<pull_t> VecPull;
typedef vector<pull_t>::iterator ItrPull;

typedef map<int, std::string> MapPullName;
typedef map<int, std::string>::iterator ItrPullName;

void  GetPullVector(ifstream &, VecPull&, VecPull&);
MapPullName GetPullMap(VecPull &);
bool IsThere(MapPullName &m, const string &s);
double Position(MapPullName &m, const string &s);
double RelativePosition(const string &c);
bool Has(const string &str, const string &sub);
void ReplaceStr(std::string& subject, const std::string& search, const std::string& replace);

void PlotPulls(){

  // open a file in read mode.
  ifstream infilePulls; infilePulls.open("./pulls.dat", ios::in); if(!infilePulls){cerr<<"Unable to open the pulls data file. "<<endl; return;}
  //ifstream infileFit; infilePulls.open("fit.dat"); if(!infileFit){cerr<<"Unable to open the fit data file. "<<endl; return;}
  string line;

  if(infilePulls.fail()) {cerr<<"fail to open the pulls file "<<endl; return;}

  //if(!infilePulls.good()) {cerr<<"pulls file not good "<<endl; return;}
  //while ( getline (infilePulls,line) ){
  //     cout <<". "<< line << '\n';
  // }


  VecPull v_pulls;
  VecPull v_norms;
  GetPullVector(infilePulls, v_pulls, v_norms);

  MapPullName m_pulls = GetPullMap( v_pulls);

  bool kGuidelines = false;
  bool kGuideboxes = true;

  bool kSigmaboxes = false;
  bool kSigmalines = true;

  double xrange = 3;
  int outerBox = 2;
  double Xrange = outerBox + outerBox*0.1;

  //pulls
  double lw = 2;
  double ms = 0.7;
  Color_t c_g = kBlack;
  Color_t c_v = kMagenta;
  Color_t c_b = kGreen;
  Color_t c_r = kAzure;

  //sigma boxes
  int col2 = 41;
  int col1 = TColor::GetColor("#cccc66");
  
  //horizontal guide boxes
  Color_t cc = kBlue-10;

  TGraphAsymmErrors *r = new TGraphAsymmErrors();
  r->SetPoint(1,  -xrange, 0);
  r->SetPoint(2,  Xrange, 0);
  r->SetMarkerSize(0);
  r->SetMarkerColor(0);
  r->SetLineWidth(0);

  TGraphAsymmErrors *g0 = new TGraphAsymmErrors();
  TGraphAsymmErrors *g1 = new TGraphAsymmErrors();
  TGraphAsymmErrors *g2 = new TGraphAsymmErrors();
  TGraphAsymmErrors *g3 = new TGraphAsymmErrors();

  int i0(1); int i1(1); int i2(1); int i3(1);
  for(ItrPull it = v_pulls.begin(); it != v_pulls.end(); it++){
    string cat = it->category;
    string nam = it->name;
    double x = it->value;
    double dxD = fabs(x -  it->errorDown);
    double dxU = fabs(x -  it->errorUp);
    if(cat == "global"){
      g0->SetPoint(i0, x, Position(m_pulls, nam) - RelativePosition(cat)); g0->SetPointError(i0, dxD, dxU, 0, 0);
      i0++;
    }else if(cat == "vbf"){
      g1->SetPoint(i1, x, Position(m_pulls, nam) - RelativePosition(cat)); g1->SetPointError(i1, dxD, dxU, 0, 0);
      i1++;
    }else if(cat == "boosted"){
      g2->SetPoint(i2, x, Position(m_pulls, nam) - RelativePosition(cat)); g2->SetPointError(i2, dxD, dxU, 0, 0);
      i2++;
    }else if(cat == "rest"){
      g3->SetPoint(i3, x, Position(m_pulls, nam) - RelativePosition(cat)); g3->SetPointError(i3, dxD, dxU, 0, 0);
      i3++;
    }else{
    }
  }

  //cout<<"Norm Factors "<<iNorm<<endl;

  double maxY = m_pulls.size();

  cout<<"Global :"<<g0->GetN()<<endl; bool k0 = g0->GetN() > 0;
  cout<<"VBF : "<<g1->GetN()<<endl; bool k1 = g1->GetN() > 0;
  cout<<"Boosted : "<<g2->GetN()<<endl; bool k2 = g2->GetN() > 0;
  cout<<"Rest :"<<g3->GetN()<<endl; bool k3 = g3->GetN() > 0;


  TCanvas *c = new TCanvas("c","",10,0,1100, 830);
  c->SetFillColor(0);
  c->SetBorderMode(0);
  c->SetBorderSize(0);
  c->SetLeftMargin(0.01);
  c->SetRightMargin(0.2);
  c->SetTopMargin(0.01);
  c->SetBottomMargin( c->GetBottomMargin()*0.5 );
  c->SetFrameBorderMode(0);
  c->SetFrameBorderMode(0);
  gPad->SetTickx();
  gPad->SetTicky();
  //gPad->SetGridy();

  c->cd();
  r->Draw("AP");
  //g->GetXaxis()->SetRangeUser(-2,2);
  r->SetMaximum(maxY);
  r->GetXaxis()->SetTitle("#left(#theta_{fit}-#theta_{0}#right)/#sigma_{#theta_{fit}}");
  r->GetXaxis()->SetLabelSize( r->GetXaxis()->GetLabelSize()*0.7 );
  r->GetYaxis()->SetLabelSize(0);
  r->GetXaxis()->SetTitleOffset(0.8);
  r->GetXaxis()->SetTitleSize(  r->GetXaxis()->GetTitleSize() * 0.7 );



  //Labels
  TLatex *lat = new TLatex();
  lat->SetTextFont(42);
  lat->SetTextSize( lat->GetTextSize() * 0.4);
  //lat->SetNDC();
  double xlat = Xrange*1.25;
  double ylat;
  for(ItrPullName it = m_pulls.begin(); it != m_pulls.end(); it++){
    //cout<<it->first<<" : "<<it->second<<endl;
    TString labely = it->second;
    labely.ReplaceAll("_", " ");
    lat->DrawText(xlat, it->first - 0.75, labely );

  }

  
  double xmin = r->GetXaxis()->GetXmin();
  double xmax = r->GetXaxis()->GetXmax();

  TBox *b1 = new TBox(-1,0,1,maxY);
  TBox *b2 = new TBox(-outerBox,0,outerBox,maxY);
  b1->SetFillColor(col1);
  b2->SetFillColor(col2);
  if(kSigmaboxes){
    b2->Draw();
    b1->Draw();
  }


  TBox *box = new TBox();
  //box->SetLineWidth(0);
  if(kGuideboxes){
    for( int j = 0; j < maxY; j = j + 2){
      //cout<<"box "<<xmin<<" "<<j<<" "<<xmax<<" "<<j+1<<endl;
      //TBox *box = new TBox(xmin, j, xmax, j+1);
      box->SetFillColor(cc); 
      //box->Draw();
      box->DrawBox(xmin, j, xmax, j+1);
    }
  }

  TLine *ls = new TLine();
  ls->SetLineWidth(2);
  ls->SetLineStyle(7);  
  if(kSigmalines){
    ls->SetLineColor(kOrange);
    ls->DrawLine(-1, 0, -1, maxY);
    ls->DrawLine(+1, 0, +1, maxY);

    ls->SetLineColor(kOrange-2);
    ls->DrawLine(-2, 0, -2, maxY);
    ls->DrawLine(+2, 0, +2, maxY);
  }

 
  TLine *l = new TLine();
  l->SetLineColor(kGray);
  l->SetLineWidth(1);
  l->SetLineStyle(7);
  if( kGuidelines){
    for( int j = 0; j < maxY; j++){
      l->SetLineColor(kGray);
      l->DrawLine(xmin, j, xmax, j);
      l->SetLineColor(0);
      l->DrawLine(-outerBox, j, outerBox, j);
    }
  }

  gPad->RedrawAxis();


  double x_left_align = 0.03;
  double x_right_align = 0.19;

  TLegend *leg = new TLegend( x_left_align, 0.7, x_right_align, 0.95);
  leg->SetFillColor(19);
  if(k0) leg->AddEntry(g0, "Global", "lp");
  if(k1) leg->AddEntry(g1, "VBF", "lp");
  if(k2) leg->AddEntry(g2, "Boosted", "lp");
  if(k3) leg->AddEntry(g3, "Rest", "lp");
  leg->Draw();

  if(k0){
    g0->SetMarkerStyle(20);
    g0->SetMarkerSize(ms);
    g0->SetMarkerColor(c_g);
    g0->SetLineColor(c_g);
    g0->SetLineWidth(lw);
    g0->Draw("P Z S");
  }

  if(k1){
    g1->SetMarkerStyle(20);
    g1->SetMarkerSize(ms);
    g1->SetMarkerColor(c_v);
    g1->SetLineColor(c_v);
    g1->SetLineWidth(lw);
    g1->Draw("P Z S");
  }

  if(k2){
    g2->SetMarkerStyle(20);
    g2->SetMarkerSize(ms);
    g2->SetMarkerColor(c_b);
    g2->SetLineColor(c_b);
    g2->SetLineWidth(lw);
    g2->Draw("P Z S");
  }

  if(k3){
    g3->SetMarkerStyle(20);
    g3->SetMarkerSize(ms);
    g3->SetMarkerColor(c_r);
    g3->SetLineColor(c_r);
    g3->SetLineWidth(lw);
    g3->Draw("P Z S");
  }

  TPad *pad = new TPad("pad","", x_left_align , 0.2,  x_right_align, 0.6, 19, 0, 0);
  double norm_range_l = 0.5;
  double norm_range_r = 1.5;
  double xAxisLevel = 0.2;
  pad->Range(norm_range_l, 0, norm_range_r, 2);
  pad->SetLeftMargin(0.02);
  pad->SetRightMargin(0.01);
  pad->SetTopMargin(0.01);
  //pad->SetBottomMargin( c->GetBottomMargin()*0.75 );
  pad->Draw();

  pad->cd();


  TGraphAsymmErrors *nf_global_z = new TGraphAsymmErrors();
  TGraphAsymmErrors *nf_global_qcd = new TGraphAsymmErrors();

  TGraphAsymmErrors *nf_vbf_z = new TGraphAsymmErrors();
  TGraphAsymmErrors *nf_vbf_qcd = new TGraphAsymmErrors();

  TGraphAsymmErrors *nf_boosted_z = new TGraphAsymmErrors();
  TGraphAsymmErrors *nf_boosted_qcd = new TGraphAsymmErrors();

  TGraphAsymmErrors *nf_rest_z = new TGraphAsymmErrors();
  TGraphAsymmErrors *nf_rest_qcd = new TGraphAsymmErrors();

  //  nf_global->SetPoint()

  //TLine *vert = new TLine(1, xAxisLevel, 1, 2);
  //vert->SetLineColor(kGray);
  //vert->SetLineStyle(7);
  //vert->Draw();

  //TGaxis *ax1 = new TGaxis(norm_range_l, xAxisLevel, norm_range_r, xAxisLevel, norm_range_l, norm_range_r, 510,"");
  //ax1->SetLabelSize( ax1->GetLabelSize() *2 );
  //ax1->Draw();

  int ig(1);
  for(ItrPull itr = v_norms.begin(); itr != v_norms.end(); itr++){
    string _cat = itr->category;
    string _name = itr->name;
    double _norm = itr->value;
    double _erR = itr->errorUp;
    double _erL = itr->errorDown;
    cout<<_cat<<" "<<_name<<" "<<_norm<<" "<<_erL<<" "<< _erR << " "<<endl;
    if ( Has(_cat, "global") ){
      if ( Has(_name, "Ztt") ) nf_global_z->SetPoint(ig, _norm, ig); nf_global_z->SetPointError(ig, fabs(_norm - _erL), fabs(_norm - _erR), 0., 0.);
      if( Has(_name, "QCD") ) nf_global_qcd->SetPoint(ig, _norm, ig); nf_global_qcd->SetPointError(ig, fabs(_norm - _erL), fabs(_norm - _erR), 0., 0.);
    }

    if ( Has(_cat, "vbf") ){
      if ( Has(_name, "Ztt") ) nf_vbf_z->SetPoint(ig, _norm, ig); nf_vbf_z->SetPointError(ig, fabs(_norm - _erL), fabs(_norm - _erR), 0., 0.);
      if( Has(_name, "QCD") ) nf_vbf_qcd->SetPoint(ig, _norm, ig); nf_vbf_qcd->SetPointError(ig, fabs(_norm - _erL), fabs(_norm - _erR), 0., 0.);
    }

    if ( Has(_cat, "boost") ){
      if ( Has(_name, "Ztt") ) nf_boosted_z->SetPoint(ig, _norm, ig); nf_boosted_z->SetPointError(ig, fabs(_norm - _erL), fabs(_norm - _erR), 0., 0.);
      if( Has(_name, "QCD") ) nf_boosted_qcd->SetPoint(ig, _norm, ig); nf_boosted_qcd->SetPointError(ig, fabs(_norm - _erL), fabs(_norm - _erR), 0., 0.);
    }

    if ( Has(_cat, "rest") ){
      if ( Has(_name, "Ztt") ) nf_rest_z->SetPoint(ig, _norm, ig); nf_rest_z->SetPointError(ig, fabs(_norm - _erL), fabs(_norm - _erR), 0., 0.);
      if( Has(_name, "QCD") ) nf_rest_qcd->SetPoint(ig, _norm, ig); nf_rest_qcd->SetPointError(ig, fabs(_norm - _erL), fabs(_norm - _erR), 0., 0.);
    }

    ig++;
  }

  double MaxY = ig+3;

  nf_global_z->SetLineColor(c_g);   nf_global_z->SetMarkerColor(c_g);  nf_global_z->SetMarkerStyle(20);
  nf_global_qcd->SetLineColor(c_g); nf_global_qcd->SetMarkerColor(c_g);  nf_global_qcd->SetMarkerStyle(24);

  nf_vbf_z->SetLineColor(c_v);   nf_vbf_z->SetMarkerColor(c_v);  nf_vbf_z->SetMarkerStyle(20);
  nf_vbf_qcd->SetLineColor(c_v); nf_vbf_qcd->SetMarkerColor(c_v);  nf_vbf_qcd->SetMarkerStyle(24);

  nf_boosted_z->SetLineColor(c_b);   nf_boosted_z->SetMarkerColor(c_b);  nf_boosted_z->SetMarkerStyle(20);
  nf_boosted_qcd->SetLineColor(c_b); nf_boosted_qcd->SetMarkerColor(c_b);  nf_boosted_qcd->SetMarkerStyle(24);

  nf_rest_z->SetLineColor(c_r);   nf_rest_z->SetMarkerColor(c_r);  nf_rest_z->SetMarkerStyle(20);
  nf_rest_qcd->SetLineColor(c_r); nf_rest_qcd->SetMarkerColor(c_r);  nf_rest_qcd->SetMarkerStyle(24);

  /*  TGraphAsymmErrors *q = new TGraphAsymmErrors();
  q->SetPoint(1, 0.5, 0);
  q->SetPoint(2, 1.5, 0);
  q->SetMarkerSize(0);
  q->SetMarkerColor(0);
  q->SetLineWidth(0);
  q->Draw("a");
  q->GetXaxis()->SetRangeUser(0.5, 1.8);*/
  TH1D *q = new TH1D("qh","", 100, 0.5, 1.55); 
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  q->Draw();
  q->SetMaximum(MaxY);
  q->GetYaxis()->SetLabelSize(0);
  q->GetYaxis()->SetTickLength(0);
  q->GetXaxis()->SetLabelSize(nf_global_z->GetXaxis()->GetLabelSize()*2 );
  
  TLine *ln = new TLine(1, 0, 1, MaxY-3); ln->SetLineWidth(1); ln->SetLineStyle(7); ln->SetLineColor(2); ln->Draw();

  nf_global_z->Draw("p s"); nf_global_qcd->Draw("p s");
  nf_vbf_z->Draw("p s"); nf_vbf_qcd->Draw("p s");
  nf_boosted_z->Draw("p s"); nf_boosted_qcd->Draw("p s");
  nf_rest_z->Draw("p s"); nf_rest_qcd->Draw("p s");

  TLegend *le = new TLegend(0.1, 0.75, 0.7, 0.97);
  le->SetFillColor(19);
  le->SetBorderSize(0);
  //le->SetHeader("NF");
  le->AddEntry(nf_global_z, "", "lp");
  le->AddEntry(nf_global_qcd, "", "lp");
  le->Draw();

  TLatex *tZ = new TLatex(0.35, 0.80, "Z#tau#tau NF"); tZ->SetNDC(); tZ->Draw(); tZ->SetTextSize( tZ->GetTextSize() * 1.75);
  TLatex *tQ = new TLatex(0.35, 0.91, "QCD NF"); tQ->SetNDC(); tQ->Draw(); tQ->SetTextSize( tQ->GetTextSize() * 1.75);
  
  return;
  /*
  double ylevel = 0.3;
  double dylevel = 0.2;
  TLine *ln = new TLine();
  ln->SetLineWidth(2);
  TEllipse *eln = new TEllipse();
  for(ItrPull itr = v_norms.begin(); itr != v_norms.end(); itr++){
    string cat = itr->category;
    if ( Has(itr->name, "Ztt") ) ln->SetLineStyle(7); else ln->SetLineStyle(1);

    if ( Has(cat, "global") ){
      ln->SetLineColor(1);
      eln->SetLineColor(1);
    } else  if ( Has(cat, "vbf") ){
      ln->SetLineColor(kAzure+1);
      eln->SetLineColor(kAzure+1);
    } if ( Has(cat, "boosted") ){
      ln->SetLineColor(kRed+1);
      eln->SetLineColor(kRed+1);
    } if ( Has(cat, "rest") ){
      ln->SetLineColor(kGreen+2);
      eln->SetLineColor(kGreen+2);
    }else{ }

    double _norm = itr->value;
    double _erR = itr->errorUp;
    double _erL = itr->errorDown;
    ln->DrawLine(_erL, ylevel, _erR, ylevel);
    //cout<<cat<<" "<<itr->name<<" "<< _erL << " "<<_norm <<" "<<_erR<<endl;
    eln->DrawEllipse( _norm, ylevel, 0.02, 0.02, 0, 365, 0, "");

    ylevel+=dylevel;
  }

  */


  infilePulls.close();
}

void  GetPullVector(ifstream &f,   VecPull &v,   VecPull &w) {

  pull_t pull;

  if(!f.good()) {cerr<<"file not good "<<endl; return;}
  string line;

  double in_pull; double in_errUp; double in_errDown; string in_cat; string in_np;
  if(f.is_open()){
    cout<<"reading file"<<endl;
    while( getline(f, line) ){

      istringstream is(line);
      is >> in_cat >> in_np >> in_pull >> in_errUp >> in_errDown;

      pull.category = in_cat; pull.name = in_np; pull.value=in_pull; pull.errorUp = in_pull + fabs(in_errUp);

      if( !Has(in_np, "norm" ) )
        pull.errorDown= in_pull + fabs(in_errDown);
      else
        pull.errorDown= in_pull - fabs(in_errDown);

      if( Has(in_np, "norm") )
        w.push_back(pull);
      else
        v.push_back(pull);
    }
  }else{
    cerr<<"cannot open file"<<endl; return;
  }

  //return v;
}

MapPullName GetPullMap(VecPull & v){

  MapPullName m;

  int idx(1);
  for(ItrPull it = v.begin(); it != v.end(); it++){
    string name = it->name;
    if( !IsThere(m, name) )
      m.insert ( std::pair<int,std::string>( idx++, name) );
  }

  return m;
}

bool IsThere(MapPullName &m, const string &s){

  for(ItrPullName it = m.begin(); it != m.end(); it++){
    if(it->second == s) return true;
  }

  return false;
}

double RelativePosition(const string &c){

  if(c=="global") return 0.2;
  if(c=="vbf") return 0.4;
  if(c=="boosted") return 0.6;
  if(c=="rest") return 0.8;

  return 0;
}

double Position(MapPullName &m, const string &s){

  int i(1);
  for(ItrPullName it = m.begin(); it != m.end(); it++){
    if(it->second == s) return i;
    i++;
  }
  cerr<<s<<" not found "<<endl;

  return 0;
}

bool Has(const string &str, const string &sub){

  std::size_t found = str.find(sub);

  if (found!=std::string::npos) return true;

  return false;
}

void ReplaceStr(std::string& subject, const std::string& search, const std::string& replace) {
  size_t pos = 0;
  while((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}
