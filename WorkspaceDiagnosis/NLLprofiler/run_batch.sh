#!/bin/sh

echo 'on machine ..'
hostname -vi

# echo "cvmfs quota ..."
# cvmfs_config showconfig | grep LIMI


cd <WHERE>
echo "in directory ..."
echo ${pwd}

SetupFile=../Setup/setup_for_ranking.sh

if [ -f $setupFile ]
  	then
	source $SetupFile    
else
	echo $SetupFile " not found"
	exit 1

fi


which root

root.exe -b -l -x "RunNLLprofiler.C(\"<NPVAR>\", \"<WSFILE>\", \"<OUTDIR>\")"
