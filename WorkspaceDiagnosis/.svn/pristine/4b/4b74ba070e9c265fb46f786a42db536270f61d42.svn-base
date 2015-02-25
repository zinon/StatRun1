#!/bin/bash

hostname -vi

shopt -s expand_aliases

export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
setupfile=${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
alias setupATLAS='source $setupfile'

type setupATLAS

echo ATLAS...
setupATLAS --quiet

echo Panda...
localSetupPandaClient --noAthenaCheck

echo VOMS...
voms-proxy-init -voms atlas
