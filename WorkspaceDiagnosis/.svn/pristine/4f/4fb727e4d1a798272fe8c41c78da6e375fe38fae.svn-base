#!/bin/sh

source ../NuisParamRepo.sh

categ=(
combination
 vbf
 boosted
#  1j_nonboosted
)

WORKDIR=$(pwd)

submit_every_seconds=0

queue=2nd

WSpath=/afs/cern.ch/work/z/zenon/Physics/Higgs/HSG4/software/hadhad/MultiVariateAnalysis/WorkspaceHadHad
Mass=125

# WSname=hh_bdt_deta_0j1j_workspace_fixed
# WSname=hh_bdt_deta_rest_workspace_fixed
# WSdir=deta_workspaces

WSdir=current
WSname=output_unblinded_random_mu



for j in "${categ[@]}"
do
	getNParray $WSdir $WSname $j
	np=(${rtr[@]})

	Outdir=$WORKDIR/results/${WSdir}_${WSname}_${j}_${queue}
	Measurement=""
    if [ "$j" = "combination" ]; then
            Measurement="combination"
        else
            Measurement=category_${j}
        fi

	echo $Measurement
   WSfile=$WSpath/$WSdir/${WSname}/xml_measurement_hh_${Measurement}_${Mass}_results/hist2workspace_combined_measurement_hh_${Measurement}_${Mass}_model.root
   if [[ ! -a $WSfile ]]; then
            echo "ERROR $WSfile does not exist!"
            return
        fi


  for i in "${np[@]}"
  do
  
    echo $i" :  "$j
    sed -e "s/<NP>/$i/g" -e "s|<OUTDIR>|$Outdir|g"  -e "s|<WSFILE>|$WSfile|g" -e "s|<WORKDIR>|$WORKDIR|g"  < RunBatchSimple.sh  > TmpRunBatchSimple.sh
#     qsub -q $queue -o ${PWD}/qsub/$i.$j.log -j oe -N $i$j TmpRunBatch.sh
	 bsub -q $queue -J $i$j -o ${PWD}/bsub/nll_%J < TmpRunBatchSimple.sh
    echo ""
    sleep $submit_every_seconds
  done
done
