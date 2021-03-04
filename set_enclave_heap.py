#!/usr/bin/env python3
#  -*- coding: UTF-8 -*-

import xml.etree.ElementTree as xml
import sys

DEFAULT_SIZE = "0x1000000"
ENCLAVE_CONF = "trusted/Enclave.config.xml"
HEAP_TAG = "HeapMaxSize"


def set_heap(size):
    xml_file = xml.ElementTree(file=ENCLAVE_CONF)
    heap_elem = xml_file.find(HEAP_TAG)

    heap_elem.text = size

    with open(ENCLAVE_CONF, 'wb') as file:
        xml_file.write(file)


if __name__ == '__main__':
    if len(sys.argv) > 1:
        try:
            size_val = int(sys.argv[1], 0)
            set_heap(sys.argv[1])
        except ValueError:
            print("Invalid value")
            sys.exit(-1)
    else:
        set_heap(DEFAULT_SIZE)
