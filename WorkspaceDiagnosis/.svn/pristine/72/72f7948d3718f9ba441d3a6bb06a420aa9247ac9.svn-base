{

	TString s = gSystem->pwd();
	
	string parentpath = s + "/../";
	
	gSystem->AddIncludePath( (" -I"+parentpath + "RooHeaders/").c_str() );
	
	gSystem->AddIncludePath( (" -I"+parentpath + "FitFunction/").c_str() );
	
	if( gSystem->Load( "../FitFunction/FitFunction_cpp.so" ) < 0 ){cerr<<"unable to load library "<<endl; exit(0);}

	gSystem->CompileMacro( "FitAndPlot.C","gk" );
	
	gROOT->ProcessLine(".q");
	

}
