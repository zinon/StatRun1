#!/bin/sh

# echo "cvmfs quota ..."
# cvmfs_config showconfig | grep LIMIT

#go to the working dir
WORKDIR=<DIR>

cd $WORKDIR

echo "working at "${PWD}

#setup
SetupMethod=<SETUP>

SetupPath=../Setup

SetupFile0=setupOpt.sh

SetupFile1=setupStd.sh

SetupFile2=cvmfs_loop.sh

SetupFile3=setup_local_HSG7.sh

cd $SetupPath

echo "setting env at "${PWD}

ls -lh

# if [ ! -f $setupFile0 ]; then
#     echo $SetupFile0 " not found"
#     trap - INT TERM EXIT
# elif [ ! -f $setupFile1 ]; then
#     echo $SetupFile1 " not found"
#     trap - INT TERM EXIT
# elif [ ! -f $setupFile2 ]; then
#     echo $SetupFile2 " not found"
#     trap - INT TERM EXIT
# elif [ ! -f $setupFile1 ]; then
#     echo $SetupFile1 " not found"
#     trap - INT TERM EXIT
# fi


case "$SetupMethod" in
"0")
	echo "Setup ATLAS with options"
	source $SetupFile0	
	;;
"1")
	echo "Setup ATLAS once"
	source $SetupFile1
	;;
"2")
	echo "Setup ATLAS with multiple tries"
	source $SetupFile2
	;;
"3")
	echo "Setup HSG7"
	source $SetupFile3
	;;
*)
	echo "Setup method $SetupMethod: do nothing"
	trap - INT TERM EXIT
	;;
esac

echo "ROOT version ..."
which root



#actual job

cd $WORKDIR/NLLscanner/

root -b -l -x <<EOF

gSystem->Load("MakeNLLProfile_C.so")

string m = "140";
string category = "<Cat>";
string path = "/home/zenon/HSG4/WorkspaceHadHad/swagato_unblinded_random_mu/";
string meusurement =  (category == "combination") ? "combination" : "category_"+category ;
string f  = "xml_measurement_hh_"+meusurement+"_"+m+"_results/hist2workspace_combined_measurement_hh_"+meusurement+"_"+m+"_model.root";

string WSf = path + f;


TFile file(WSf.c_str());
if (file.IsZombie()) {
  cout << "Error opening file " <<WSf<< endl;
  exit(0);
}else{
  cout<<"Using file "<<WSf<<endl;
}

string NPname  = "<NP>";

string outdir = "NLL_swagato_unblinded_random_mu_"+m+"_" + category;

cout<<outdir<<endl;

int Npoints = <Npoints>; 
float NPmin = -2.5; 
float NPmax = 2.5;

MakeNLLProfile(WSf, outdir, NPname, Npoints, NPmin, NPmax);

.q
EOF
