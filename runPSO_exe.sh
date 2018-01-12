#!/bin/bash

set -e

if [ "$#" -eq 1 ]; then

  ODIR="$1"

else

  printf "\n>>> ERROR -- invalid list of command-line argument(s):\n"
  printf   "             [1] path to output directory\n\n"
  exit

fi

if [ -d "${ODIR}" ]; then

  printf "\n>>> ERROR -- target output directory already exists: ${ODIR}\n\n"
  exit

fi

###

mkdir -p "${ODIR}"

for i_categ in `seq 5 7`; do

  nohup ./runPSO.py \
    -c     PSOConfig_boostedHbb_step7_cate"${i_categ}".txt \
    -o "${ODIR}"/PSO_boostedHbb_step7_cate"${i_categ}" \
    >& "${ODIR}"/PSO_boostedHbb_step7_cate"${i_categ}".log &

done
unset -v i_categ

###

unset -v ODIR
