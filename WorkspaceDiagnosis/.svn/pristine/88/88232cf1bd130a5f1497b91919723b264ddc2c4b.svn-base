{

	TString s = gSystem->pwd();
	
	string parentpath = s + "/../";
	
	string fullpath = parentpath + "RooHeaders/";
		
	gSystem->AddIncludePath( (" -I"+fullpath).c_str() );
	
	gSystem->CompileMacro( "FitFunction.cpp","gk" );

	gROOT->ProcessLine(".q");
}
