#!/usr/bin/bash
COREID=0
PROGRAM=sgx_perf

set_governor() {
  echo "$1" | sudo tee /sys/devices/system/cpu/cpu${COREID}/cpufreq/scaling_governor
}

set_governor "performance"
taskset -c ${COREID} ./${PROGRAM}
set_governor "powersave"

