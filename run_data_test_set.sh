#!/bin/bash
BEGIN_BYTES=250
ITER_COUNT=5000
MAX_BYTES=300000000

STEP_VAL=250
CUR_STEP=0
STEP=2
STEP_MULT=4

echo "Start time: `date`"
./run_perf_testing.sh ${1}/sgx_test_const_dest.txt $ITER_COUNT
echo "Saving to ${1}/sgx_test_const_dest.txt"
echo

for (( i = ${BEGIN_BYTES} ; i <= ${MAX_BYTES}  ; i += ${STEP_VAL})); do
  ./run_perf_testing.sh "${1}/sgx_test_${i}_bytes.txt" $ITER_COUNT $i
  echo "Saving to ${1}/sgx_test_${i}_bytes.txt"
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
