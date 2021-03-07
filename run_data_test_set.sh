#!/bin/bash
BEGIN_BYTES=250
ITER_COUNT=5000
ITER_COUNT_CONST=2000
MAX_BYTES=300000000

STEP_VAL=250
CUR_STEP=0
STEP=2
STEP_MULT=4

update_enc_size() {
  sed -i "s/#define DATA_SIZE .*/#define DATA_SIZE ${1}lu/" "untrusted/sgx_perf.h"
  make update_empty > /dev/null
}

fix_size_filename() {
  sed -i "s/| Data size:.*/| Data size:      ${i}\t|/" "${1}/size_var/sgx_test_${i}_bytes.txt"
}

make

echo "Start time: `date`"
echo

mkdir -p "${1}/data_var/"
mkdir -p "${1}/size_var/"

for (( i = ${BEGIN_BYTES} ; i <= ${MAX_BYTES}  ; i += ${STEP_VAL})); do
  ./run_perf_testing.sh "${1}/data_var/sgx_test_${i}_bytes.txt" $ITER_COUNT $i
  echo "Saving to ${1}/data_var/sgx_test_${i}_bytes.txt"

  update_enc_size ${i}
  ./run_perf_testing.sh "${1}/size_var/sgx_test_${i}_bytes.txt" ${ITER_COUNT_CONST}
  echo "Saving to ${1}/size_var/sgx_test_${i}_bytes.txt"

  echo
  if [[ ${CUR_STEP} -ge ${STEP} ]]; then
    ((STEP_VAL *= STEP_MULT))
    CUR_STEP=0
    i=0
  else
    ((CUR_STEP += 1))
  fi
done

echo "Finish time: `date`"
