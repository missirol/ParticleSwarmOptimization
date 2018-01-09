#!/bin/bash

ODIR="$1"

mkdir -p "${ODIR}"

for i_categ in `seq 0 0`; do

#  nohup ./runPSO.py --dryrun \
  nohup ./runPSO.py \
    -c     PSOConfig_boostedHbb_step7_cate"${i_categ}".txt \
    -o "${ODIR}"/PSO_boostedHbb_step7_cate"${i_categ}" \
    >& "${ODIR}"/PSO_boostedHbb_step7_cate"${i_categ}".log &

done
unset -v i_categ

unset -v ODIR
