# SGX Test
Testing software for Intel SGX

## Requirements
* Intel SGX Driver
* Intel SGX SDK and PSW
* *KVM and QEMU SGX Linux patches required for virtualization support*

## Build
* Run <code> make </code>

## Run
* Run a specific number of repeats at a given data length
  * Run <code> ./run_perf_testing.sh {result_set_filename} {repeat_count} {data_len} </code>
  * {result_set_filename}: the filename to save the test results to
  * {repeat_count}: the number of times to repeat a test
  * {data_len}: the length of the data packet to test, set to 0 to run Enclave size tests

* Run a set number of tests in the system
  * Run <code> ./run_data_test_set.sh {result_set_folder} </code>
  * {result_set_folder}: the output folder for the result set

## Graph creation
*Python 3, Numpy and Matplotlib is required*
* Run <code> ./create_graphs.py </code>
