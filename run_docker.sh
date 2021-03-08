#!/bin/bash

sudo docker build -t enclave_test .
sudo docker run --device /dev/isgx --device /dev/mei0 enclave_test
