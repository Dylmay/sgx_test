#!/usr/bin/bash
COREID=0
PROGRAM=sgx_perf
RESULT_LOC=output.txt

set_governor() {
  echo "$1" | sudo tee /sys/devices/system/cpu/cpu${COREID}/cpufreq/scaling_governor
}

set_governor "performance"
echo "Running test..."
taskset -c ${COREID} ./${PROGRAM} > "$RESULT_LOC"
echo "Test finished. Results saved to $RESULT_LOC"
set_governor "powersave"

