/* 

October 2013, Goettingen

Zinonas Zinonos, zenon@cern.ch

Description:
===============
This class can be used with any ROOT file prepared in the standard way for fit model studies. The following arguments need to be specified:
- name for input ROOT file
- name of workspace inside ROOT file that holds model and data
- name of ModelConfig that specifies details for calculator tools
- name of dataset 

The code runs over all channels and the input list of nuisance parameters and constructs for each a representation of -log(L) of a fitting PDF with given dataset (createNLL). 
For a binned dataset, a binned likelihood is also constructed.

Then it creates a RooProfileLL object that eliminates all nuisance parameters in the given function using the RooFit's "createProfile". 
The nuisance parameters are defined as all parameters of the function except the stated Parameter of Interest (in our case is the signal mu)


References:
===============
- https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/NuisanceCheck
- http://root.cern.ch/root/html/tutorials/roostats/
- http://root.cern.ch/drupal/content/roofit
*/


#ifndef _NLLPROFILER_
#define _NLLPROFILER_

#include <iostream> 
#include <cmath> 
#include <string>
#include <map>
#include <sstream>
using namespace std;

#include "TLatex.h"
#include "TFile.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TString.h"

#include "RooHeaders.h"

#include "FitFunction.h"

class FitFunction;

struct CurveFitFunction { 
	CurveFitFunction(RooCurve *f) : 
	fFunc(f) 
	{
		//constructor
	}
	
	double operator() (double *x, double *)  const { 
		return fFunc->Eval( x[0] );
	}
	
	RooCurve * fFunc; 
};

class NLLprofiler  {
	
	public:
		NLLprofiler();
		~NLLprofiler();

		void SetInputFileName(const string &s){m_input_file_name=s;}
		void SetOutputDirectoryName(const string &s){m_output_dir_name=s;}
		void SetOutputFileName(const string &s){m_outfile_name=s;}
		void SetWorkspaceName(const string &s){m_ws_name=s;}
		void SetModelConfigName(const string &s){m_confmod_name=s;}
		void SetObsDataName(const string &s){m_obsdfata_name=s;}
		void Print_ModelObservables(){ m_print_mc_obs = true; }
		void Print_NuisanceParameters(){ m_print_nps = true; }
		void Print_NuisanceParametersList(){ m_print_nps_list = true; }
		void Print_SubChannels(){m_print_subchan = true; }
		void Print_FitPDF_InitialParams(){m_print_fit_pdf_init_params = true;}
		void Print_FitPDF_Model(){ m_print_fit_pdf_model = true;}
		void SetRooFitMsgLvl(RooFit::MsgLevel i){ m_roofit_msg_lvl = i;}
		void SetRooMinimizerPrintLvl(int i){ m_RooMinimizer_print_lvl = i;}
		void SetPOI(double mu){m_poi = mu;}
		void Minos(){ m_minos = true;} 
		void Minuit2(){ m_minimizer = "Minuit2"; }
		void Strategy(int i){m_strategy=i;}
		void MaxFitRetries(int r){m_max_fit_retries=r;}
		void ParallelProcessing(unsigned int i){m_n_cpus = i; cout<<"NLLprofiler - parallel processing with "<<m_n_cpus<<" cpus"<<endl;}
		void FitSubChannels(){ m_fit_subchannels = true;}
		void ScanVariable(const string &s){ m_scan_variable = s;}
		void ScanPOI(){m_scan_poi = true;}
		void ScanGreen(){ m_scan_green = true;}
		void Calculate(bool do_the_job);
		
	private:
		string GetInputFileName(){return m_input_file_name;};
		string GetOutputDirectoryName(){return m_output_dir_name;};
		string GetWorkspaceName(){return m_ws_name;};
		string GetModelConfigName(){return m_confmod_name;};
		string GetObsDataName(){return m_obsdfata_name; };
		
		bool 	SetProperties();
		TFile 	*GetWorkspaceFile(){return m_ws_file;}
		void 	PrintModelObservables();
		void 	PrintNuisanceParameters();
		void 	PrintNuisanceParametersList();
		void 	PrintSubChannels();
		void 	PrintNumberOfEventsPDF(RooAbsPdf *pdf);
		bool 	StrHasSubstr(const string &str, const string &sub);
		bool 	NormFactor(RooRealVar *var);
		bool 	ContructNPmap();
		bool 	CheckSimultaneousPDF();
		RooCategory *GetCategories();
		RooSimultaneous *GetSimPDF();
		bool 	CheckChannelName();
		void 	SetFitConditional(){m_fit_conditional = true;}
		bool 	LoadSnapshot(const string &snapshot);
		void 	SetFirstPOI();
// 		RooFitResult* FitPDF( ModelConfig* model, RooAbsPdf* fitpdf, RooAbsData* fitdata, int &MinuitStatus, int &HessStatus, double &Edm, TString minimType, bool useMinos );
		void 	Profile(RooAbsReal *nll, RooRealVar *var, bool floating, const string &channelName);
		string  GetParabolaMeanAndErrorsString(RooCurve *rc);
		string  char2str(char *c){ stringstream ss; string s;  ss << c; ss >> s; return s; }
		
	private:

		static const string m_snapshot_initial_name;
		static const double m_nnl_range_low, m_nnl_range_high;
		
		RooFit::MsgLevel m_roofit_msg_lvl;
		int m_RooMinimizer_print_lvl;
		string m_outfile_name;
		bool m_fit_conditional;
		bool m_fit_subchannels;
		bool m_print_mc_obs;
		bool m_print_nps;
		bool m_print_nps_list;	
		bool m_print_subchan;
		int m_strategy;
		double m_poi;
		string m_minimizer; 
		unsigned int m_max_fit_retries;
		unsigned int m_n_cpus;
		bool m_minos;
		bool m_print_fit_pdf_init_params;
		bool m_print_fit_pdf_model;
		string m_scan_variable;
		bool m_scan_poi;
		bool m_scan_green;
		double m_LumiRelError;
		string m_input_file_name;
		string m_output_dir_name;
		string m_ws_name;
		string m_confmod_name;
		string m_obsdfata_name;
		
		
		TFile *m_outfile;
		TFile *m_ws_file;
		RooWorkspace *m_ws;
		ModelConfig *m_mc;
		RooAbsData *m_data;
		RooSimultaneous* m_pdf;
		RooArgSet *m_parameters;
		RooArgSet *m_observables;
		RooArgSet *m_nuisance;
		RooRealVar *m_firstPOI;
		map<string, double> m_NP_map;
		
		FitFunction *m_fit_function;
		
	public:
		ClassDef(NLLprofiler, 0)
		
};
#endif
