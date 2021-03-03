#!/bin/bash
ITER_COUNT=10000
MAX_BYTES=1000000000
INCREMENT=1024

for (( i = ${INCREMENT} ; i < ${MAX_BYTES}  ; i += ${INCREMENT})); do
  echo "Saving to ${1}/sgx_test_${i}_bytes.txt"
  ./run_perf_testing.sh "${1}/sgx_test_${i}_bytes.txt" $ITER_COUNT $i
done
