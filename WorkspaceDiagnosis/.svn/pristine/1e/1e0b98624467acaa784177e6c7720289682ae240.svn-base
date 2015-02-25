#include "FitFunction.h"

///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
FitFunction::FitFunction() :
 m_print_fit_pdf_model(false),
 m_print_fit_pdf_init_params(false),
 m_RooMinimizer_print_lvl(0)
{
	   
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
FitFunction::~FitFunction()
{
	
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
RooFitResult* FitFunction::FitPDF( ModelConfig* model, RooAbsPdf* fitpdf, RooAbsData* data, int &MinuitStatus, int &HessStatus, double &Edm, TString minimType, bool useMinos, unsigned int maxRetries, unsigned int nCPUs) {
	//definition of the fit function
	
	cout<<"========================================================================================"<<endl;
	cout << "FitFunction::FitPDF - starting" <<endl; TStopwatch sw; sw.Start(); 
	cout<<"========================================================================================"<<endl;
	
	if(m_print_fit_pdf_model){
		cout << "Initial model parameters" <<endl;
		model->Print();	
	}
	
	RooArgSet* constrainedParams = fitpdf->getParameters(*data);
	RemoveConstantParameters(constrainedParams);
	Constrain(*constrainedParams);
	
	const RooArgSet* glbObs = model->GetGlobalObservables();
	
	RooRealVar * poi = (RooRealVar*) model->GetParametersOfInterest()->first();
	cout << "POI const = " << poi->isConstant() << " value = "<< poi->getVal() <<endl;
	
	RooAbsReal * nll = nCPUs ? fitpdf->createNLL(*data, Constrain(*constrainedParams), GlobalObservables(*glbObs), Offset(1), NumCPU(nCPUs) ) :
	fitpdf->createNLL(*data, Constrain(*constrainedParams), GlobalObservables(*glbObs), Offset(1));
	double nllval = nll->getVal();
	
	if(m_print_fit_pdf_init_params){
		cout << "Initial pdf parameters" <<endl;
		constrainedParams->Print("v");
	}
	
	cout << "Initial NLL = " << nllval << endl;
	
	static unsigned  int nrItr = 0;
	ROOT::Math::MinimizerOptions::SetDefaultMinimizer(minimType);
	//ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(m_RooMinimizer_print_lvl);
	int strat = ROOT::Math::MinimizerOptions::DefaultStrategy();
	cout<<"Fit strategy chosen : "<<strat<<endl;
	
	int save_strat = strat;
	RooMinimizer minim(*nll);
	minim.setStrategy(strat);
	minim.setPrintLevel(m_RooMinimizer_print_lvl);
	minim.setEps(1);

	int status=-99;
	HessStatus=-99;
	Edm = -99;
	RooFitResult * r;
	while (nrItr < maxRetries && status!=0 && status!=1){

		cout << "Fit try n°" << nrItr+1 << " ( max = " << maxRetries << " )" << endl;		
		
		ROOT::Math::MinimizerOptions::SetDefaultStrategy(save_strat);
		status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(),ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
		HessStatus= minim.hesse();
		r = minim.save();
		Edm = r->edm();
		
		//up the strategy
		bool FitIsNotGood = ((status!=0 && status!=1) || (HessStatus!=0 && HessStatus!=1) || Edm>1.0);
		if (FitIsNotGood && strat<2){
			cout << endl;
			cout << "   *******************************" << endl;
			cout << "   * Increasing Minuit strategy (was " << strat << ")" << endl;
			strat++;
			cout << "   * Fit failed with : " << endl;
			cout << "      - minuit status " << status << endl;
			cout << "      - hess status " << HessStatus << endl;
			cout << "      - Edm = " << Edm << endl;
			cout << "   * Retrying with strategy " << strat << endl;
			cout << "   ********************************" << endl;
			cout << endl;
			minim.setStrategy(strat);
			status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
			HessStatus= minim.hesse();
			r = minim.save();
			Edm = r->edm();
		}
		
		FitIsNotGood = ((status!=0 && status!=1) || (HessStatus!=0 && HessStatus!=1) || Edm>1.0);
		if (FitIsNotGood && strat < 2){
			cout << endl;
			cout << "   ********************************" << endl;
			cout << "   * Increasing Minuit strategy (was " << strat << ")" << endl;
			strat++;
			cout << "   * Fit failed with : " << endl;
			cout << "      - minuit status " << status << endl;
			cout << "      - hess status " << HessStatus << endl;
			cout << "      - Edm = " << Edm << endl;
			cout << "   * Retrying with strategy " << strat << endl;
			cout << "   ********************************" << endl;
			cout << endl;
			minim.setStrategy(strat);
			status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
			HessStatus= minim.hesse();
			r = minim.save();
			Edm = r->edm();
		}
		
		FitIsNotGood = ((status!=0 && status!=1) || (HessStatus!=0 && HessStatus!=1) || Edm>1.0);
		if (FitIsNotGood && strat < 2){
			cout << endl;
			cout << "   *******************************" << endl;
			cout << "   * Increasing Minuit strategy (was " << strat << ")" << endl;
			strat++;
			cout << "   * Fit failed with : " << endl;
			cout << "      - minuit status " << status << endl;
			cout << "      - hess status " << HessStatus << endl;
			cout << "      - Edm = " << Edm << endl;
			cout << "   * Retrying with strategy " << strat << endl;
			cout << "   ********************************" << endl;
			cout << endl;
			minim.setStrategy(strat);
			status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
			HessStatus= minim.hesse();//added
			r = minim.save();
			Edm = r->edm();
		}
		
		if(useMinos) { minim.minos(); }
		
		
		FitIsNotGood = ((status!=0 && status!=1) || (HessStatus!=0 && HessStatus!=1) || Edm>1.0);
		if ( FitIsNotGood) nrItr++;
		if (nrItr == maxRetries) {
			cout << endl;
			cout << "***********************************************************" << endl;
			cout << "WARNING::Fit failure unresolved with status " << status << endl;
			cout << "   Please investigate your workspace" << endl;
			cout << "   Find a wall : you will need it to crash your head on it" << endl;
			cout << "***********************************************************" << endl;
			cout << endl;
			MinuitStatus = status;
			return r;
		}
		
	}  
	
	r = minim.save();
	cout << endl;
	cout << "***********************************************************" << endl;
	cout << "         FIT FINALIZED SUCCESSFULLY : " << endl;
	cout << "            - minuit status " << status << endl;
	cout << "            - hess status " << HessStatus << endl;
	cout << "            - Edm = " << Edm << endl;
	cout << "***********************************************************" << endl;
	cout << endl;
	
	
	MinuitStatus = status;
	
	cout<<"========================================================================================"<<endl;
	cout << "FitFunction::FitPDF - finihed.  Time : "; sw.Print();
	cout<<"========================================================================================"<<endl;
	
	return r;
}

