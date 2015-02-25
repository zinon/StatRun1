#!/bin/sh

source ../NuisParamRepo.sh


DRY=false

WSdir=Run1PaperHadhad_MVA
WSname=r160507
WSforNuisRepo=$WSname
Scenario=""
Category=combination

WSfolder=${WSdir}_${WSname}

getNParray $WSdir $WSforNuisRepo $Category ${Scenario}
NPlist=(${rtr[@]})

WSFILE=/afs/cern.ch/work/z/zenon/Physics/Higgs/HSG4/software/hadhad/MultiVariateAnalysis/WorkspaceHadHad/Run1PaperHadhad2/hh_combination_125/xml_measurement_hh_combination_125_results/hist2workspace_combined_measurement_hh_combination_125_model.root


if [[ ! -a $WSFILE ]]; then
	echo "ERROR : $WSFILE does not exist!"
    return
fi

THISDIRE=${PWD}

echo $THISDIRE
THISHOST=$(hostname)
THISCASE=0
THISQUEUE=none

if [ "$THISHOST" == "rocks.local" ]; then
        THISQUEUE=atlas
	THISCASE=1
elif grep -q lxplus <<<$THISHOST; then
    THISQUEUE=8nh
	THISCASE=2
fi

OUTDIR=${WSfolder}_${THISQUEUE}

echo "Output directory " $OUTDIR


echo "NP size ${#NPlist[@]}"

for i in "${NPlist[@]}"
do
  echo "submitting "$i
  sed -e "s/<NPVAR>/$i/g" -e "s|<WSFILE>|$WSFILE|g" -e "s|<OUTDIR>|$OUTDIR|g"  -e "s|<WHERE>|$THISDIRE|g" < run_batch.sh > tmp_run_batch.sh


  case "$THISCASE" in
      "1")
      if $DRY; then
	  echo qsub -q $THISQUEUE -o ${PWD}/qsub/$i.log -j oe -N $i tmp_run_batch.sh
      else
	  qsub -q $THISQUEUE -o ${PWD}/qsub/$i.log -j oe -N $i tmp_run_batch.sh
      fi
      ;;

      "2")
      if $DRY; then
		echo bsub -q $THISQUEUE -o ${PWD}/bsub/nll_%J -J $i < tmp_run_batch.sh
      else
		bsub -q $THISQUEUE  -o ${PWD}/bsub/nll_%J -J $i < tmp_run_batch.sh
      fi
      ;;
  
      *)
      echo "unable to send job to batch"
      ;;
  esac

  shift;

done
