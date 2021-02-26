#!/usr/bin/env python3
#  -*- coding: UTF-8 -*-

import json
from matplotlib import pyplot
import numpy

OUTPUT_FILENAME = "output.txt"
HEADER_INDICATOR = "--"


class Test:
    """ Testing types """
    READ  = "Read"
    WRITE = "Write"
    CONST = "Construct"
    DEST  = "Destruct"
    IN    = "Input"
    OUT   = "Output"


def to_dict(file) -> dict:
    """ Converts an SGX testing result set into a dict """
    output_dict = {}
    current_header = None

    for line in file:
        if HEADER_INDICATOR in line:
            current_header = line.strip("- \n").split(" ")[0].capitalize()
            output_dict.setdefault(current_header, [])
            continue

        if current_header:
            if line.strip() != "":
                recording_result = line.split(";")[1].strip("\n ns")
                output_dict[current_header].append(int(recording_result))

    return output_dict


def average_results(dct: dict):
    """ Averages the results held in a test result dict """
    averages = {}

    for key, value in dct.items():
        averages.setdefault(key, sum(value) / len(value))

    return averages


def draw_bar_chart_read(result_set, colour='g'):
    """ Creates a bar chart displaying read measurements """
    pyplot.title("Read performance")
    pyplot.ylabel("time taken (ns)")

    __finalise_chart(fetch_test_results(result_set, Test.READ), colour)


def draw_bar_chart_write(result_set, colour='g'):
    """ Creates a bar chart displaying write measurements """
    pyplot.title("write performance")
    pyplot.ylabel("time taken (ns)")

    __finalise_chart(fetch_test_results(result_set, Test.WRITE), colour)


def draw_bar_chart_const(result_set, colour='g'):
    """ Creates a bar chart displaying construct measurements """
    pyplot.title("Construct performance")
    pyplot.ylabel("time taken (ns)")

    __finalise_chart(fetch_test_results(result_set, Test.CONST), colour)


def draw_bar_chart_dest(result_set, colour='g'):
    """ Creates a bar chart displaying destruct measurements """
    pyplot.title("Destruct performance")
    pyplot.ylabel("time taken (ns)")

    __finalise_chart(fetch_test_results(result_set, Test.DEST), colour)


def draw_bar_chart_input(result_set, colour='g'):
    """ Creates a bar chart displaying input measurements """
    pyplot.title("Input performance")
    pyplot.ylabel("time taken (ns)")

    __finalise_chart(fetch_test_results(result_set, Test.IN), colour)


def draw_bar_chart_output(result_set, colour='g'):
    """ Creates a bar chart displaying output measurements """
    pyplot.title("Output performance")
    pyplot.ylabel("time taken (ns)")

    __finalise_chart(fetch_test_results(result_set, Test.OUT), colour)


def __finalise_chart(data, colour):
    """ Helper function used to finish off the created bar charts """
    systems = tuple(data.keys())
    index = numpy.arange(len(systems))
    bar_width = 0.4

    pyplot.bar(index, data.values(), color=colour, width=bar_width)
    pyplot.xticks(index, systems)
    pyplot.show()


def fetch_test_results(result_set: dict, test_type):
    """ Fetches a test result from a result set """
    data_bundle = {}

    for key, result in result_set.items():
        data_bundle.setdefault(key, result.get(test_type.capitalize()))

    return data_bundle


if __name__ == '__main__':
    output_file = open(OUTPUT_FILENAME)
    sgx_driver_results = to_dict(output_file)
    results = {}

    results.setdefault("SGX Driver", average_results(sgx_driver_results))
    results.setdefault("SGX KVM", average_results(sgx_driver_results))
    results.setdefault("Virt SGX", average_results(sgx_driver_results))

    draw_bar_chart_const(results)
    draw_bar_chart_dest(results)
