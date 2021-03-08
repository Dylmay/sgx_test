#!/bin/bash
COREID=0
PROGRAM=sgx_perf

help() {
  echo "Intel SGX performance testing script"
  echo "usage: ./run_perf_testing.sh {result filename} {repeat count} {data_len}"
}

set_governor() {
  echo "$1" | sudo tee /sys/devices/system/cpu/cpu${COREID}/cpufreq/scaling_governor
}

if [ $# -eq 0 ]; then
  help
  exit
fi

TEST_STR="Running test..."

if [ $# -gt 1 ]; then
  TEST_STR+=" count=$2 repeats;"
fi
if [ $# -gt 2 ]; then
  TEST_STR+=" data_len=$3 bytes"
fi

source /opt/intel/sgxsdk/environment
set_governor "performance"
echo "$TEST_STR"
taskset -c ${COREID} ./${PROGRAM} $2 $3 > "$1"
echo "Test finished. Results saved to $1"
set_governor "powersave"

