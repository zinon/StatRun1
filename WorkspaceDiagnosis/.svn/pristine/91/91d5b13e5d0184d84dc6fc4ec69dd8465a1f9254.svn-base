/*

Fit function definition

*/
#ifndef _FITFUNCTION_
#define _FITFUNCTION_

#include <iostream> 
using namespace std;

//ROOT
#include "TROOT.h"
#include "TH2D.h"


#include "RooHeaders.h"


class FitFunction  {
	
	public:
		FitFunction();
		FitFunction(bool show_pdf_model, bool show_pdf_par, int print_lvl){
			m_print_fit_pdf_model = show_pdf_model;
			m_print_fit_pdf_init_params = show_pdf_par;
			m_RooMinimizer_print_lvl = print_lvl;
		};
		~FitFunction();

		
		RooFitResult* FitPDF( ModelConfig* model, RooAbsPdf* fitpdf, RooAbsData* fitdata, int &MinuitStatus, int &HessStatus, double &Edm, TString minimType, bool useMinos, unsigned int maxItrRetries, unsigned int nCPUs );
		
		
	private:
		bool m_print_fit_pdf_model;
		bool m_print_fit_pdf_init_params;
		int  m_RooMinimizer_print_lvl;
		
	public:
		ClassDef(FitFunction, 0)
		
};
#endif
