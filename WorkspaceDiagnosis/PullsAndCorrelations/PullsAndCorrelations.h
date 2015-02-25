/*


*/
#ifndef _PULLSANDCORRELATIONS_
#define _PULLSANDCORRELATIONS_

#include <iostream> 
using namespace std;

//ROOT
#include "TROOT.h"
#include "TH2D.h"
#include "TFile.h"
#include "TSystem.h"

#include "RooHeaders.h"

#include "FitFunction.h"

class FitFunction;

class PullsAndCorrelations  {
	
	public:
		PullsAndCorrelations();
		~PullsAndCorrelations();
		
		void SetInputFileName(const string &s){m_input_file_name=s;}
		void SetOutputDirectoryName(const string &s){m_output_dir_name=s;}
		void SetOutputFileName(const string &s){m_outfile_name=s;}
		void SetWorkspaceName(const string &s){m_ws_name=s;}
		void SetModelConfigName(const string &s){m_confmod_name=s;}
		void SetObsDataName(const string &s){m_obsdfata_name=s;}
		void SetRooFitMsgLvl(RooFit::MsgLevel i){ m_roofit_msg_lvl = i;}
		void SetRooMinimizerPrintLvl(int i){ m_RooMinimizer_print_lvl = i;}
		void SetPOI(double mu){m_poi = mu;}
		void Minos(){ m_minos = true;} 
		void Minuit2(){ m_minimizer = "Minuit2"; }
		void Strategy(int i){m_strategy=i;}
		void MaxFitRetries(int r){m_max_fit_retries=r;}
		void ParallelProcessing(unsigned int i){m_n_cpus = i; cout<<"NLLprofiler - parallel processing with "<<m_n_cpus<<" cpus"<<endl;}
		void FitSubChannels(){ m_fit_subchannels = true;}
		void Calculate();
		
		
	private:
		string GetInputFileName(){return m_input_file_name;};
		string GetOutputDirectoryName(){return m_output_dir_name;};
		string GetWorkspaceName(){return m_ws_name;};
		string GetModelConfigName(){return m_confmod_name;};
		string GetObsDataName(){return m_obsdfata_name; };
		
		void 	CorrelationMatrix(const string &fit, RooFitResult *rfr);
		bool 	SetProperties();
		TFile 	*GetWorkspaceFile(){return m_ws_file;}
		bool 	CheckSimultaneousPDF();
		bool 	CheckChannelName();
		void 	SetFitConditional(){m_fit_conditional = true;}
		bool 	LoadSnapshot(const string &snapshot);
		void 	SetFirstPOI();
		RooCategory *GetCategories();
		RooSimultaneous *GetSimPDF();
		
		bool StrHasSubstr(const string &str, const string &sub);
		
	private:
		
		static const string m_snapshot_initial_name;
		
		RooFit::MsgLevel m_roofit_msg_lvl;
		int m_RooMinimizer_print_lvl;
		string m_outfile_name;
		bool m_fit_conditional;
		bool m_fit_subchannels;
		int m_strategy;
		double m_poi;
		string m_minimizer; 
		unsigned int m_max_fit_retries;
		unsigned int m_n_cpus;
		bool m_minos;
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
// 		map<string, double> m_NP_map;
		
		FitFunction *m_fit_function;

	public:
		ClassDef(PullsAndCorrelations, 0)
		
};
#endif
