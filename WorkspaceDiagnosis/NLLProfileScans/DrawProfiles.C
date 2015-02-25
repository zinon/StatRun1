// https://svnweb.cern.ch/trac/atlasinst/browser/Institutes/Freiburg/Higgs/StatTools/trunk/macros

#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TF1.h"


#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <utility>      // std::pair, std::make_pair
#include <fstream>
#include <algorithm>
#include <sstream>
#include <ctime>

using namespace std;

bool Has(const string &str, const string &sub);

void DrawNNLscan(const string &np, const string &cat, const string &folder, TFile *fout, bool draw, bool kMakeEPS);

void DrawDependencies(const string &np1, const string &np2, const string &cat, const string &folder, TFile *fout, bool draw, bool kMakeEPS, double rndmu);

TTree *GetNPtree(const string &np, const string &folder, int &n);
TTree *GetNPtreeFromFile(const string &name);

string rmSpaces(const string &str);

void printvector (string i);

bool Filters(vector<string> &v, const string &s);

string DetectNP(const string &s, char del);

string DetectCat(const string &s, char del);

string DetectExactCat(const string &s, char del);

string  char2str(char *c);

string GetParabolaMeanAndErrorsString(TGraph *rc);
string GetMeanAndErrorsString(double errL, double mean, double errR);
string GetStringFromInt(int n);

string DetectCategory(const string &str, vector<string> &v);

template <typename OutIt> OutIt split(const std::string &text, char sep, OutIt out);


struct CurveFitFunction {
  CurveFitFunction(TGraph *f) :
    fFunc(f)
  {
    //constructor
  }

  double operator() (double *x, double *)  const {
    return fFunc->Eval( x[0] );
  }

  TGraph * fFunc;
};

void DrawProfiles()
{

  srand( (unsigned)time(0) );
  double mu_rndm = (float)rand()/(float)RAND_MAX;

  mu_rndm = 0;

  bool kInfo              = true;
  bool kDrawPlots         = false;
  bool kMakeEPS           = false;
  bool kMakeNLL           = true;
  bool kDepend            = false;
  bool kTestCode          = false;
  string  NPquest         = "mu";

  string kind = "grid";
  string workspace = "Run1PaperHadhad_MVA_r160507";
  string mass = "";
  string tag = "Apr28";

  string outputfile = ( kMakeNLL ? "NLLscans." : kDepend ? "NLLdep." : "test") + workspace
    + ( mass.size() ? "." + mass : "")
    + ( tag.size() ? "."+tag : "");

  //filters
  vector<string> filters;
  //filters.push_back(workspace);
  if(mass.size()) filters.push_back(mass);
  if(tag.size()) filters.push_back(tag);
  filters.push_back("NLLscans");
  filters.push_back("root");
  for(vector<string>::iterator is = filters.begin(); is != filters.end(); is++)
    cout<<"filter "<<*is<<endl;

  vector<string> cats;
  cats.push_back("combination");
  //cats.push_back("combination.2011");
  //cats.push_back("combination.2012");
  //cats.push_back("vbf");
  //cats.push_back("boosted");
  //      cats.push_back("1j_nonboosted");
 for(vector<string>::iterator c = cats.begin(); c != cats.end(); c++)
   cout<<"cat "<<*c<<endl;

  ///open file and create the list of NP's: find grid_downloads/Nov11/ -name "*.root*" > filelists/output_unblinded_tes_mfs_sym_Nov11_grid_files.txt
 // find grid_downloads/Apr28/ -name "*.root*" > filelists/Run1PaperHadhad_MVA_r160507_Apr28_grid_files.txt 

  ifstream in_stream;

  string infile = "filelists/"+workspace + "_" + tag + "_" +  kind + "_files.txt";

  in_stream.open(infile.c_str());
  if (! in_stream.is_open())  { cerr << "cant' open file "+infile+"\n";  exit(1); }else{cout<<"reading file: "<<infile<<endl;}
  vector<string> list;
  string line;
  while(!in_stream.eof() /*&& in_stream.good()*/ )
    {
      in_stream >> line;
      line = rmSpaces(line);
      //cout<<"reading "<<line<<endl;
      if(line.size()) if( Filters( filters, line)) list.push_back( line );
    }
  if(kInfo) cout<<"Number of NP selected : "<<list.size()<<endl;
  //if(kInfo) { cout<<"List of Files "<<endl; for_each (list.begin(), list.end(), printvector); }

  ///output file
  TFile *fout = new TFile( ( "plots/" + outputfile + ".root").c_str(), "recreate");

  //loop over NPs
  for(vector<string>::iterator it = list.begin(); it != list.end(); it++) {

    string NP, categ;
    if(kind == "grid"){
      char delim('.');       NP = DetectNP(*it, delim);      categ = DetectExactCat(*it, delim);
    }else{
      string s1 = "ScanAndFit_"; string s2 = "_fixed";
      NP = *it; NP.erase(0, NP.find(s1) + s1.size() ); NP.erase(NP.find(s2));
      categ = DetectCategory(*it, cats);
    }
 
    if(NP.empty()) {cerr<<"Could not recognize *NP* for file "<< *it <<endl; continue;}

    if(categ.empty()) {cerr<<"Could not recognize *category* for file "<< *it <<endl; continue;}

    //draw plot
    if(kMakeNLL){//NLL
      DrawNNLscan( NP, categ, *it, fout, kDrawPlots, kMakeEPS);

    } else if(kDepend){ //dependencies
      if(NP != NPquest)
        DrawDependencies( NP, NPquest, categ,  *it, fout, kDrawPlots, kMakeEPS, mu_rndm);

    } else{
      cout<<"dry run for "<<*it<<endl;
    }

  }//loop

  list.clear();
  in_stream.close();

  cout<<"random "<<mu_rndm<<endl;
  cout<<"Done"<<endl;

}

//--------------------------------------------------------------------------------------------------------
TTree *GetNPtree(const string &np, const string &folder, int &n){

  if(np.empty()) {cerr<<"GetNPtree - empty np string "<<endl; exit(0);}

  string file = folder+"/ScanAndFit_"+np+"_fixed2.5_2.5.root";

  TFile *f = new TFile( file.c_str() );

  if(!f){ cerr<<"unable to read root file "<<file<<endl; return NULL; }

  TTree *t = (TTree*)f->Get("FitResults");

  if(!t) { cerr<<"unable to get tree "<<endl; return NULL; }

  n = t->GetEntries();

  if(!n) { cerr<<"empty tree "<<endl; return NULL; } else { cout<<"entries "<<n<<endl;}

  return t;

}
//--------------------------------------------------------------------------------------------------------
TTree *GetNPtreeFromFile(const string &name){

  if(name.empty()) {cerr<<"empty file name "<<endl; return NULL;}

  TFile *f = new TFile( name.c_str() );

  if(!f){ cerr<<"unable to read root file "<<name<<endl; return NULL; }

  TTree *t = (TTree*)f->Get("FitResults");

  if(!t) { cerr<<"unable to get tree "<<endl; return NULL; }

  int n(0); n = t->GetEntries();

  if(!n) { cerr<<"empty tree "<<endl; return NULL; } else { cout<<"entries "<<n<<endl;}

  return t;
}
//--------------------------------------------------------------------------------------------------------
void DrawDependencies(const string &NP1, const string &NP2, const string &cat, const string &filename, TFile *fout, bool kDrawPlots, bool kMakeEPS, double mu_rndm){

  if(NP1.empty()) {cerr<<"empty np1 string "<<endl; exit(0);}

  if(NP2.empty()) {cerr<<"empty np2 string "<<endl; exit(0);}

  cout<<NP1<<" Vs "<<NP2<<endl;

  bool isBlinded = false;

  int n(0);
  TTree *t = GetNPtreeFromFile(filename);//GetNPtree(NP, folder + "_" + cat, n);
  if(!t) {cerr<<"skipping "<<NP1<<" vs "<<NP2<<" in "<<cat<<" ..."<<endl; return;}
  n = t->GetEntries();
  if(!n){cerr<<"tree with no entries "<<endl; return;}

  int col = 1;
  bool kMarker = true;

  if(cat == "vbf"){
    kMarker = false;
    col = kRed+1;
  }

  if(cat == "boosted"){
    kMarker = false;
    col = kAzure+1;
  }

  if(cat == "1j_nonboosted"){
    kMarker = false;
    col = kGreen+1;
  }

  string np1 = NP1 == "SigXsecOverSM" ? "mu" : NP1;
  string np2 = NP2 == "SigXsecOverSM" ? "mu" : NP2;

  bool isMu(false);
  if( np2 =="mu") isMu = true;

  double        np1_val;
  double        np1_err;
  int           np1_const_val;

  double        np2_val;
  double        np2_err;
  int np2_const_val;

  unsigned int          MinuitStatus;


  t->SetBranchAddress( np1.c_str(),                             &np1_val);
  t->SetBranchAddress( (np1+"_err").c_str(),    &np1_err);
  t->SetBranchAddress( (np1+"_const").c_str(),  &np1_const_val);

  t->SetBranchAddress( np2.c_str(),                             &np2_val);
  t->SetBranchAddress( (np2+"_err").c_str(),    &np2_err);

  t->SetBranchAddress( "MinuitStatus",  &MinuitStatus);

  double x[n], y[n], dxl[n], dxh[n], dyl[n], dyh[n], dx[n], dy[n];

  vector<int> remP;

  for(int i = 0; i< n; i++){
    int e = t->GetEntry(i);

    if( MinuitStatus <= 1 && np1_const_val > 0.5 ){
      x[i] = np1_val;
      y[i] = np2_val;
      double f = 1.0;
      dxl[i] = np1_err/f;
      dxh[i] = np1_err/f;
      dyl[i] = np2_err/f;
      dyh[i] = np2_err/f;

      dx[i] = 0;//np1_err;
      dy[i] = np2_err;

      if(isMu) {
        //cout<< y[i] << " + " << mu_rndm << endl;
        y[i]+=mu_rndm;
      }


      //cout<<left<<setw(3)<<i<<". "<<x[i]<<" "<<dxl[i]<<" "<<dxh[i]<<" : "<<y[i]<<" "<<dyl[i]<<" "<<dyh[i]<<endl;
    }else{
      x[i] = 0;
      y[i] = 0;
      dxl[i] = 0;
      dxh[i] = 0;
      dyl[i] = 0;
      dyh[i] = 0;
      remP.push_back(i);
    }

  }

  TGraphErrors *gr = new TGraphErrors(n, x, y, dx, dy );
  //    for(int i = 0; i <n; i++){
  //            gr->SetPoint(i, x[i], y[i]);
  //            gr->SetPointError(i, dx[i], dy[i])
  //    }

  //TGraphAsymmErrors *gr = new TGraphAsymmErrors(n, x, y, dxl, dxh, dyl, dyh);

  for(vector<int>::iterator i=remP.begin(); i != remP.end(); i++) gr->RemovePoint(*i);

  gr->SetTitle();
  gr->SetLineColor(col);
  gr->SetFillColor(col);
  gr->SetFillStyle(3003);
  gr->SetMarkerColor(col); cout<<col<<"  "<<cat<<endl;
  if(kMarker) gr->SetMarkerStyle(4);
  gr->GetXaxis()->SetTitle( np1.c_str() );
  gr->GetYaxis()->SetTitle( np2.c_str() );
  if(kDrawPlots) gr->Draw("APZ");
  if(isMu)
    if(isBlinded)
      gr->GetYaxis()->SetRangeUser( mu_rndm - 0.01, mu_rndm+0.05);
    else
      gr->GetYaxis()->SetRangeUser( 0.7, 1.3);

  string gname = "dep_"+cat+"_"+np1+"_vs_"+np2;

  gr->SetName( gname.c_str() );

  fout->cd();
  gr->Write();
  fout->Save();


  if(kMakeEPS)
    gr->SaveAs( ("eps/"+gname+".eps").c_str() );

}
//--------------------------------------------------------------------------------------------------------
void DrawNNLscan(const string &NP, const string &cat, const string &filename, TFile *fout, bool kDrawPlots, bool kMakeEPS){


  if(NP.empty()) {cerr<<"empty np string "<<endl; exit(0);}

  cout<<"\nscanning "<<NP<<" in "<<cat<<" ..."<<endl;

  int n(0);
  TTree *t = GetNPtreeFromFile(filename);//GetNPtree(NP, folder + "_" + cat, n);
  if(!t) {cerr<<"skipping "<<NP<<" in "<<cat<<" ..."<<endl; return;}
  n = t->GetEntries();
  if(!n){cerr<<"tree with no entries "<<endl; return;}

  int col = 1;
  bool kMarker = true;

  if(cat == "combination11"){
    kMarker = false;
    col = kRed+1;
  }

  if(cat == "combination12"){
    kMarker = false;
    col = kAzure+1;
  }

  if(cat == "vbf"){
    kMarker = false;
    col = kRed+1;
  }

  if(cat == "boosted"){
    kMarker = false;
    col = kAzure+1;
  }

  if(cat == "1j_nonboosted"){
    kMarker = false;
    col = kGreen+1;
  }

  double nll_val;
  double np_val;
  int np_const_val;
  unsigned int  MinuitStatus;

  bool nllErrorFromMedian = false;

  string gname = "nll_prof_"+cat+"_"+NP;

  string np = NP == "SigXsecOverSM" ? "mu" : NP;

  t->SetBranchAddress( "nll_val",       &nll_val);
  t->SetBranchAddress( np.c_str(),      &np_val);
  t->SetBranchAddress( (np+"_const").c_str(),   &np_const_val);
  t->SetBranchAddress( "MinuitStatus",  &MinuitStatus);

  typedef std::vector < std::pair <double, double> > TPairs;
  typedef std::vector < std::pair <double, double> >::iterator TPairsItr;

  double status_thresh = 1;
  int n0 = np =="mu" ? 20 : 0;
  TPairs pairs;
  //loop
  double xMinVal;
  cout<<"loop in range "<<n0<<" to "<<n<<endl;
  for(int i = n0; i< n; i++){
    int e = t->GetEntry(i);

    if (!np_val) cerr << np << " couldn't get np val at "<<i<< endl;

    if (!nll_val) cerr<< np << " couldn't get nll val at "<<i<< endl;

    if( MinuitStatus <= status_thresh && ( np_const_val > 0.5 || np == "mu" ) )
      pairs.push_back( make_pair(np_val, nll_val));

    if( np != "mu" && MinuitStatus <= status_thresh && np_const_val < 0.5 )
      cout << np << " min value found at "<<np_val<<endl;

  }


  if(pairs.empty()){cerr<<"NNL scan - empty profile, zero points -> no plots will be saved"<<endl; return;}

  //iterate over values and create graph
  const int npoints = pairs.size();
  cout<<"graph size "<<npoints<<endl;
  TGraph *g = new TGraph( npoints );
  int ipoint = 0;
  double        minNllVal = 1e6;
  double        minNpVal;
  TPairsItr itr = pairs.begin();
  double lError, rError;
  while(itr != pairs.end() ){
    //cout<<itr->first<<" "<<itr->second<<endl;
    double dnll = 2 * itr->second;
    double x = itr->first;
    g->SetPoint(ipoint, x,  dnll);


    if( itr->second < minNllVal){
      minNllVal = itr->second; minNpVal =  itr->first;
    }

    itr++; ipoint++;
  }
  itr = pairs.begin();
  while(itr != pairs.end() ){
    double x = itr->first;
    double dnll = 2 * itr->second;
    //cout<<x<<" : "<<dnll<<endl;
    if( fabs(dnll - 1) < 0.05 ){
      if(x < minNpVal) lError = x;
      else rError = x;
    }
    itr++;
  }

  double dlError = - fabs(minNpVal - lError);
  double drError = + fabs(minNpVal - rError);

  cout<<"Minimum "<<minNpVal<<" "<< dlError<<" "<<drError<<endl;

  g->SetTitle();
  g->SetName( gname.c_str() );
  if(kDrawPlots) g->Draw("apl");
  if(kMarker) g->SetMarkerStyle(4);
  g->SetMarkerColor(col);
  g->SetLineColor(col);
  g->GetXaxis()->SetTitle( np.c_str() );
  g->GetYaxis()->SetTitle( "2^{}#Delta^{}[-log(L)] ");

  string parabola = nllErrorFromMedian ?
    "#color["+GetStringFromInt(col)+"]{" +GetParabolaMeanAndErrorsString(g)+"}"
    :
    "#color["+GetStringFromInt(col)+"]{" +GetMeanAndErrorsString(dlError, minNpVal, drError)+"}";

  g->SetTitle(parabola.c_str());



  fout->cd();
  cout<<"Saving graph : "<<gname<<endl;
  g->Write();
  fout->Save();

  if(kMakeEPS) g->SaveAs( ("eps/"+gname+".eps").c_str() );
}

string GetMeanAndErrorsString(double errL, double mean, double errH){

  char mean_char[12];
  char errL_char[12];
  char errH_char[12];
  sprintf(mean_char,"%.2f", mean);
  sprintf(errL_char,"%.2f", errL);
  sprintf(errH_char,"%.2f", errH);


  return char2str(mean_char) + "^{+" + char2str(errH_char) + "}_{-" + char2str(errL_char) + "}";
}
string GetParabolaMeanAndErrorsString(TGraph *rc){

  CurveFitFunction *fitf = new CurveFitFunction(rc);
  TF1 f("f", fitf, -2.5, 2.5, 0, "FitFunction");
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
//--------------------------------------------------------------------------------------------------------
string rmSpaces(const string &str){
  string temp;
  for (unsigned int i = 0; i < str.length(); i++)
    if (str[i] != ' ') temp += str[i];

  return temp;
}

//--------------------------------------------------------------------------------------------------------
void printvector (string i) {
  std::cout << i <<endl;
}
//--------------------------------------------------------------------------------------------------------
bool Has(const string &str, const string &sub){

  std::size_t found = str.find(sub);

  if (found!=std::string::npos) return true;

  return false;
}
//--------------------------------------------------------------------------------------------------------
bool Filters(vector<string> &v, const string &s){

  for(vector<string>::iterator it = v.begin(); it != v.end(); it++){
    if( !Has(s, *it) ) return false;
  }
  return true;
}

template <typename OutIt> OutIt split(const std::string &text, char sep, OutIt out)
{
  size_t start = 0, end=0;

  while((end = text.find(sep, start)) != std::string::npos)
    {
      *out++ = text.substr(start, end - start);
      start = end + 1;
    }

  *out++ = text.substr(start);

  return out;
}
string DetectCategory(const string &str, vector<string> &v){


  for(vector<string>::iterator it = v.begin(); it != v.end(); it++){
    if (str.find(*it) != string::npos) return *it;
  }

  return "";
}

string DetectCat(const string &s, char del){

  vector<string> tokens;

  split(s, del, std::back_inserter(tokens));

  for(vector<string>::iterator it = tokens.begin(); it != tokens.end(); it++)
    if( Has(*it, "combination") ||  Has(*it, "vbf") || Has(*it, "boosted") || Has(*it, "rest") ) return *it;

  return "";

}

string DetectExactCat(const string &s, char del){

  //vector<string> tokens;

  //split(s, del, std::back_inserter(tokens));

  //for(vector<string>::iterator it = tokens.begin(); it != tokens.end(); it++){
    if( Has(s, "combination.2011") ) return "combination11";
    if( Has(s, "combination.2012") ) return "combination12";
    if( Has(s, "combination") ) return "combination";
    if( Has(s, "vbf11") ) return "vbf11";
    if( Has(s, "vbf12") ) return "vbf12";
    if( Has(s, "vbf") ) return "vbf";
    if( Has(s, "boosted11") ) return "boosted11";
    if( Has(s, "boosted12") ) return "boosted12";
    if( Has(s, "boosted") ) return "boosted";
    if( Has(s, "rest11") ) return "rest11";
    if( Has(s, "rest12") ) return "rest12";
    if( Has(s, "rest") ) return "rest";
    //}

  return "";

}

string DetectNP(const string &s, char del){

  vector<string> tokens;

  split(s, del, std::back_inserter(tokens));

  for(vector<string>::iterator it = tokens.begin(); it != tokens.end(); it++)
    if( Has(*it, "alpha") ||  Has(*it, "norm") || Has(*it, "SigXsecOverSM") ) return *it;

  return "";

}

string  char2str(char *c){ stringstream ss; string s;  ss << c; ss >> s; return s; }
string GetStringFromInt(int n){ std::ostringstream stm; stm << n; return stm.str(); }

