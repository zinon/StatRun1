#include "PullsAndCorrelations.h"

//ClassImp(PullsAndCorrelations); 


const string PullsAndCorrelations::m_snapshot_initial_name = "snapshot_paramsVals_initial";

///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
PullsAndCorrelations::PullsAndCorrelations() :
m_roofit_msg_lvl(RooFit::INFO),
m_RooMinimizer_print_lvl(1),
m_outfile_name("test"),
m_fit_conditional(false),
m_fit_subchannels(false),
m_strategy(0),
m_poi(0),
m_minimizer("Minuit"), 
m_max_fit_retries(1),
m_n_cpus(0),
m_minos(false),
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
PullsAndCorrelations::~PullsAndCorrelations()
{
	m_outfile->Save();
	m_outfile->Close();
	
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void PullsAndCorrelations::Calculate(){
	
	if (! SetProperties() ){cerr<<"ERROR Stop. PullsAndCorrelations::Calculate - unable to set properties"<<endl; exit(0); }
		
	CorrelationMatrixGlobal();
	
	CorrelationMatrixSubChannels();
	
	
	///correlation matrix per category
	
	//set all params to their initial values
	if( !LoadSnapshot(m_snapshot_initial_name) ) {cerr<<"ERROR Stop. PullsAndCorrelations::Calculate - couldn't upload "<<m_snapshot_initial_name<<endl; exit(0); } 
	
	
	
	cout<<"\nSuccess"<<endl;
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void PullsAndCorrelations::CorrelationMatrixGlobal(){
	
	//set all params to their initial values
	if( !LoadSnapshot(m_snapshot_initial_name) ) {cerr<<"ERROR Stop. PullsAndCorrelations::Calculate - couldn't upload "<<m_snapshot_initial_name<<endl; exit(0); } 
	
	//set first POI
	SetFirstPOI();
	
	//get roo sim pdf
	RooSimultaneous *simPdf = GetSimPDF();
	
	//check first POI
	if(m_fit_conditional) m_firstPOI->setConstant();
	
	//
	RooMsgService::instance().setGlobalKillBelow(m_roofit_msg_lvl);//enum MsgLevel { DEBUG=0, INFO=1, PROGRESS=2, WARNING=3, ERROR=4, FATAL=5 } ; RooFit::WARNING
	
	//minimization
	ROOT::Math::MinimizerOptions::SetDefaultStrategy(m_strategy);
	int status_Minuit 	= -1234;
	int status_HESSE 	= -1234;
	double EDM			= -1234;
	RooFitResult  *globalfit = m_fit_function->FitPDF( m_mc, simPdf, m_data, status_Minuit, status_HESSE, EDM, m_minimizer, m_minos, m_max_fit_retries, m_n_cpus );
	
	const RooArgSet *ParamsGlobalFit = m_mc->GetNuisanceParameters();
	m_ws->saveSnapshot("snapshot_paramsVals_GlobalFit", *ParamsGlobalFit);
	
	double muhat = m_firstPOI->getVal();
	cout<<"PullsAndCorrelations::Calculate() - muhat = "<<muhat<<endl;
	m_firstPOI->setConstant(false);
	
	CorrelationMatrix("Combined", globalfit);
	
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void PullsAndCorrelations::CorrelationMatrixSubChannels(){
	
	//set all params to their initial values
	if( !LoadSnapshot(m_snapshot_initial_name) ) {cerr<<"ERROR Stop. PullsAndCorrelations::Calculate - couldn't upload "<<m_snapshot_initial_name<<endl; exit(0); } 
	
	//set first POI
	SetFirstPOI();
	
	//get roo sim pdf
	RooSimultaneous *simPdf = GetSimPDF();
	
	RooCategory* cats = (RooCategory*) (&simPdf->indexCat());
	TIterator *iter = cats->typeIterator() ;
	RooCatType *tt  = NULL;
	while((rct=(RooCatType*) itr->Next()) ){
		string SubChannelName(rct->GetName());
		// Retrieve pdf and datset associated to the current channel
		RooAbsPdf  *pdf_tmp  = simPdf->getPdf( rct->GetName() );
		RooArgSet  *obs_tmp  = pdftmp->getObservables( m_mc->GetObservables() ) ;
		RooAbsData *data_tmp = m_data->reduce(Form("%s==%s::%s",cats->GetName(), cats->GetName(), rct->GetName()));
		RooRealVar *obs = ((RooRealVar*) obs_tmp->first());
		
		//check first POI
		if(m_fit_conditional) m_firstPOI->setConstant();
		
		//minimization
		RooMsgService::instance().setGlobalKillBelow(m_roofit_msg_lvl);//enum MsgLevel { DEBUG=0, INFO=1, PROGRESS=2, WARNING=3, ERROR=4, FATAL=5 } ; RooFit::WARNING
		
		ROOT::Math::MinimizerOptions::SetDefaultStrategy(m_strategy);
		int status_Minuit 	= -1234;
		int status_HESSE 	= -1234;
		double EDM			= -1234;
		RooFitResult  *catfit = m_fit_function->FitPDF( m_mc, simPdf, m_data, status_Minuit, status_HESSE, EDM, m_minimizer, m_minos, m_max_fit_retries, m_n_cpus );
		
		double muhat = m_firstPOI->getVal();
		m_firstPOI->setConstant(kFALSE);
		
		CorrelationMatrix(SubChannelName, catfit);
	}



}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool PullsAndCorrelations::SetProperties(){
	
	if(m_minos) m_n_cpus = 0;
	
	//output dir
	gSystem->Exec( ("mkdir -p " + GetOutputDirectoryName() ).c_str() );
	if( ! gSystem->OpenDirectory( GetOutputDirectoryName().c_str() ) ){ cerr<<"PullsAndCorrelations::SetProperties - couldn't create directory "<<GetOutputDirectoryName()<<endl; return false; }
	
	//outputfile
	m_outfile = new TFile( (GetOutputDirectoryName() + m_outfile_name +".root").c_str(), "recreate");
	
	//input file
	m_ws_file = TFile::Open( (GetInputFileName()).c_str() );
	if (!m_ws_file) { cerr << "PullsAndCorrelations::SetProperties - unable to open the file " << GetInputFileName() << endl; return false;	}
	Long_t id,size,flags,mt;
	gSystem->GetPathInfo( GetInputFileName().c_str(),&id,&size,&flags,&mt);
	if( size < 10000 ){	 cerr << "PullsAndCorrelations::SetProperties - trying to open a file with too few bytes" << GetInputFileName() << " " << size << endl; return false; }
	if( m_ws_file->TestBit(TFile::kRecovered) ){cerr << "PullsAndCorrelations::SetProperties - trying to open an unclosed file " << GetInputFileName() << endl; return false;}
	if( m_ws_file->IsZombie() ){ cerr << "PullsAndCorrelations::SetProperties - zombie file " << GetInputFileName() << endl; return false;	}
	
	//workaspace
	m_ws      = (RooWorkspace*) m_ws_file->Get( GetWorkspaceName().c_str() );
	if(!m_ws) {cerr << "PullsAndCorrelations::SetProperties - unable to obtain WS " << GetWorkspaceName() << "from file "<< GetInputFileName() <<endl; return false;}
	
	//model config
	m_mc = (ModelConfig*) m_ws->obj( GetModelConfigName().c_str() );
	if(!m_mc) {cerr << "PullsAndCorrelations::SetProperties - unable to obtain MC " << GetModelConfigName() << "from WS "<< GetWorkspaceName() <<endl; m_ws->Print(); return false;}
	
	//data
	m_data = (RooAbsData*)m_ws->data( GetObsDataName().c_str() );
	if(!m_data) {cerr << "PullsAndCorrelations::SetProperties - unable to retrieve data " << GetObsDataName() << "from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}
	
	//pdf
	string m_pdf_name = "simPdf";
	m_pdf = (RooSimultaneous*) m_ws->pdf(m_pdf_name.c_str());
	if(!m_pdf) {cerr << "PullsAndCorrelations::SetProperties - unable to retrieve PDF " <<  m_pdf_name<< "from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}
	
	//parameters
	m_parameters = (RooArgSet*) m_pdf->getParameters(*m_data) ;
	if(!m_parameters) {cerr << "PullsAndCorrelations::SetProperties - unable to load data parameters from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}
	
	//observables
	m_observables = (RooArgSet*) m_mc->GetObservables();
	if(!m_observables) {cerr << "PullsAndCorrelations::SetProperties - unable to get all observables from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}
	
	//nuisance params
	m_nuisance = (RooArgSet*)m_mc->GetNuisanceParameters();
	if(!m_nuisance) {cerr << "PullsAndCorrelations::SetProperties - unable to get NPs from WS "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}
	
	//load initial snapshot
	if( !LoadSnapshot(m_snapshot_initial_name) ){
		if( m_ws->saveSnapshot(m_snapshot_initial_name.c_str(), *m_parameters) ){
			cout<<"PullsAndCorrelations::SetProperties - snapshot "<< m_snapshot_initial_name <<" is created and stored"<<endl;
		}else{
			cerr<<"PullsAndCorrelations::SetProperties - couldn't create and save snapshot '"<<m_snapshot_initial_name<<"'"<<endl; return false;
		}
	}else{ 
		cout<<"PullsAndCorrelations::SetProperties - Snapshot "+m_snapshot_initial_name+" already exists in  workspace "+GetWorkspaceName()+" and it will not be overwritten" << endl;
	}
	
	//check sim pdf
	if(! CheckSimultaneousPDF() ) { cerr << "PullsAndCorrelations::SetProperties - problems with simultaneous PDF " <<endl; m_ws->Print(); return false; }
	
	//check channel name
	if(! CheckChannelName() ){cerr << "PullsAndCorrelations::SetProperties - problems with the channel name in "<< GetWorkspaceName() <<endl;  m_ws->Print(); return false;}
	
	
	//the fitting PDF
	m_fit_function = new FitFunction();
	if(!m_fit_function){ cerr<<"PullsAndCorrelations::SetProperties - couldn't create the fit PDF class object"<<endl; return false;}
	
	return true;
}
// Get the RooSimultaneous PDF
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void PullsAndCorrelations::CorrelationMatrix(const string &fit, RooFitResult *rfr){
	
	TH2D *h = (TH2D*)rfr->correlationHist();
	string htitle = fit;
	string hname = "h_CorrelationMatrix_"+fit;
	h->SetName(hname.c_str());
	h->SetTitle(htitle.c_str());
	
	m_outfile->cd();
	h->Write();
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
void PullsAndCorrelations::SetFirstPOI(){
	
	m_firstPOI = dynamic_cast<RooRealVar*>(m_mc->GetParametersOfInterest()->first());
	m_firstPOI->setVal(m_poi);
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
RooCategory *PullsAndCorrelations::GetCategories(){
	
	return (RooCategory*) & GetSimPDF()->indexCat();
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
RooSimultaneous *PullsAndCorrelations::GetSimPDF(){
	
	return (RooSimultaneous*)m_mc->GetPdf();
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool PullsAndCorrelations::LoadSnapshot(const string &snapshot){
	
	return m_ws->loadSnapshot( snapshot.c_str() );
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool PullsAndCorrelations::CheckSimultaneousPDF(){
	
	if (strcmp(m_mc->GetPdf()->ClassName(),"RooSimultaneous") ){
		cout << " ERROR : no Simultaneous PDF was found, must check all input histograms." << endl;
		return false;
	}
	
	return true;
}
///<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
bool PullsAndCorrelations::CheckChannelName(){
	
	// 	RooSimultaneous* simPdf = (RooSimultaneous*)(mc->GetPdf());
	
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
bool PullsAndCorrelations::StrHasSubstr(const string &str, const string &sub){
	
	std::size_t found = str.find(sub);
	
	return found!=std::string::npos;
	
}


