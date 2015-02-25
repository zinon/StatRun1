#!/bin/bash
# localSetupPandaClient --noAthenaCheck

source NuisParamRepo.sh


CategArray=(
     combination
      vbf
       boosted
#  rest
)

Tag=Nov11
Mass=125
# 0BV
Scenario=""
# swagato_unblinded_random_mu 
# hh_bdt_deta_rest_workspace_fixed
# hh_bdt_deta_rest_workspace_fixed

WSdir=current
#WSname=output_unblinded_random_mu
WSname=output_unblinded_tes_mfs_sym
WSforNuisRepo=output_unblinded

Npoints=200
rangeL=2.5
rangeR=2.5


local=false
dry=false
oneJob=true
oneNuis=SigXsecOverSM
submit_every_seconds=0.5
nGBPerJob=MAX
RootVersion=5.34.10
Cmt=x86_64-slc5-gcc43-opt
WorkDir=${PWD}
SitesBlackList=UKI-SCOTGRID-ECDF_SCRATCHDISK,IEPSAS-KOSICE_SCRATCHDISK,RRC-KI_SCRATCHDISK,BNL-OSG2_USERDISK,NET2_USERDISK,TUDRESDEN-ZIH_SCRATCHDISK,BNL-OSG2_USERDISK,UAM-LCG2_SCRATCHDISK,IN2P3-CC_SCRATCHDISK,TRIUMF-LCG2_SCRATCHDISK,DESY-ZN_SCRATCHDISK,UAM-LCG2_SCRATCHDISK

case $local in
  (false)   type -P prun &>/dev/null && { echo "prun set"; } || { echo "prun is not set. Try: localSetupPandaClient --noAthenaCheck; voms-proxy-init -voms atlas "; return;}
;;
esac

ijob=0
for j in "${CategArray[@]}"
do
    Category=$j
    getNParray $WSdir $WSforNuisRepo $Category
    NpArray=(${rtr[@]})
    totsize=$(( ${#CategArray[@]} * ${#NpArray[@]} ))
    catsize=${#NpArray[@]}

    for i in "${NpArray[@]}"
    do
        NP=$i

        (( ijob++ ))
        echo $ijob / ${totsize}, $catsize

        OutputRoot=ScanAndFit_${NP}_fixed${rangeL}_${rangeR}.root

        Measurement=""

        if [ "$Category" = "combination" ]; then
            Measurement="combination"
        else
            Measurement=category_${Category}
        fi

		WSpath="workspaces/${WSdir}/${WSname}"
        if [ -n "$Scenario" ]; then
            OutputDS=user.zenon.NLLscans.${WSname}.${Mass}.${NP}.${Category}.${Scenario}.${Tag}
            WSfolder=$WSpath/xml_measurement_hh_${Measurement}_${Mass}_${Scenario}_results
        else
            OutputDS=user.zenon.NLLscans.${WSname}.${Mass}.${NP}.${Category}.${Tag}
            WSfolder=$WSpath/xml_measurement_hh_${Measurement}_${Mass}_results
        fi

        [  -d "${WSfolder}" ] || { echo "WS folder '$WSfolder' does not exist ...";  return; }

        WSfilecore=hist2workspace_combined_measurement_hh_${Measurement}_${Mass}_model
        WSfile=${WSfilecore}.root
        WSrootfile=../${WSfolder}/${WSfile}
	
###
        cd ${WSfolder}
		if [[ ! -a $WSfile ]]; then
            echo "ERROR $WSfile does not exist!"
            return
        fi

        [ -f "${WSfilecore}.ntup" ] || { echo "creating ntup ...";  cp $WSfile ${WSfilecore}.ntup; }
        cd $WorkDir

        WSrootfileGrid=${WSfolder}/${WSfilecore}.ntup

        if [[ ! -a $WSrootfileGrid ]]; then
            echo "ERROR $WSrootfileGrid does not exist!"
            return
        fi
###

        sed -e "s|<WS>|${WSrootfile}|g" \
            -e "s/<NP>/${NP}/g" \
            -e "s/<Npoints>/$Npoints/g" \
            -e "s/<Lrange>/-$rangeL/g" \
            -e "s/<Rrange>/$rangeR/g" \
            < NLLProfileScans/RunCode.C   > NLLProfileScans/TmpRunCode.C

        sed -e "s|<FOLDER>|${WSfolder}|g" \
            -e "s|<EXPECTEDFILE>|${WSfilecore}|g" \
            < RunJob.sh > TmpRunJob.sh

	if $oneJob; then
	   if [ "$NP" != "$oneNuis" ]; then
	     continue
	   fi
	fi

        if $dry; then
            echo "dry run: $WSname : $NP :  $Category ... "
            continue
        elif $local; then
            echo "running locally: $WSname : $NP :  $Category ... "
            source TmpRunJob.sh
        else
            echo "submitting: $WSname : $NP :  $Category ... "
            prun --exec "TmpRunJob.sh" \
                --excludeFile=\*.log,\*.o,\*.so,\*.a,\*.d,\*.tmp,\*.dat,\*.tex,\*.svn,\*.log,\*.txt,\*.png,\*.pdf,\*.eps,\*.ps,\*tex*,\*.root,\*.table,\core.*,*\*.*~,\png,\bsub,\prun,\LSFJOB_* \
                --outDS=$OutputDS \
                --outputs=$OutputRoot \
                --rootVer=$RootVersion \
                --cmtConfig=$Cmt \
                --workDir=$WorkDir \
                --extFile=$WSrootfileGrid \
				--excludedSite=$SitesBlackList \
                --crossSite=200  > prun/job.$OutputDS.log 2>&1
            sleep $submit_every_seconds
        fi

    done

    [ -f $WSrootfileGrid ] && { echo "removing $WSrootfileGrid ..."; rm -f $WSrootfileGrid; }
done



#                       --excludeFile=\*.log,\*.o,\*.so,\*.a,\*.d,\*.tmp,\*.dat,\*.tex,\*.svn,\*.table,\*.log,\*.txt,\*.root,\*.tgz,\*.tar,\*.gz,\*.*~ \
