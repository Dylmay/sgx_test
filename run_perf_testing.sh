#!/usr/bin/bash
COREID=0
PROGRAM=sgx_perf

help() {
  echo "Intel SGX performance testing script"
  echo "usage: ./run_perf_testing.sh {result filename}"
}

set_governor() {
  echo "$1" | sudo tee /sys/devices/system/cpu/cpu${COREID}/cpufreq/scaling_governor
}

if [ $# -eq 0 ]; then
  help
  exit
fi

set_governor "performance"
echo "Running test..."
taskset -c ${COREID} ./${PROGRAM} > "$1"
echo "Test finished. Results saved to $1"
set_governor "powersave"

