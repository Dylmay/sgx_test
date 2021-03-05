#!/usr/bin/env python3
#  -*- coding: UTF-8 -*-
import json

from matplotlib import pyplot
import numpy

import os

HEADER_INDICATOR = "--"
TRACE_INDICATOR = "#"
SEPARATOR = "_"


class Setting:
    INDICATOR = '|'
    DATA_LEN = "Data len"
    DATA_SIZE = "Data size"
    ENCLAVE = "Enclave .so"
    ITER = "Count"


class Test:
    """ Testing types """
    READ = "Read"
    WRITE = "Write"
    CONST = "Construct"
    DEST = "Destruct"
    IN = "Input"
    OUT = "Output"
    ENC = "Encryption"
    DEC = "Decryption"


class Stat:
    """ Stat types """
    MEAN = "mean"
    UPPER = "upper"
    LOWER = "lower"
    STD_DEV = "std dev"
    VAR = "variance"


class TestData(dict):
    """ Class used to hold stats from a dataset """

    def __init__(self, readings: list):
        super().__init__(TestData.__create_stats(readings))

    def get_stat(self, stat_type: str) -> int:
        return self.get(stat_type)

    @staticmethod
    def __create_stats(readings: list) -> dict:
        return {
            Stat.MEAN: numpy.nanmean(readings),
            Stat.UPPER: max(readings),
            Stat.LOWER: min(readings),
            Stat.STD_DEV: numpy.nanstd(readings),
            Stat.VAR: numpy.nanvar(readings)
        }


class ResultSet(dict):
    """ Class used to represent data taken from testing """

    def __init__(self, filename: str):
        super().__init__()
        self.__construct_result_set(filename)

    def __construct_result_set(self, filename: str):
        """ Constructs/inits the result set """
        current_header = None
        readings = []

        file = open(filename)

        for line in file:
            if HEADER_INDICATOR in line:
                if current_header:
                    self.setdefault(current_header, TestData(readings))
                current_header = line.strip("- \n").split(" ")[0].capitalize()
                readings = []

                continue

            if current_header:
                if line.strip(" \n" + SEPARATOR) != "":
                    if HEADER_INDICATOR and TRACE_INDICATOR not in line:
                        recording_result = line.split(";")
                        readings.append(sec_to_nsec(int(recording_result[1].strip("\n (ns)")))
                                        + int(recording_result[2].strip("\n (ns)")))

        if current_header and readings != []:
            self.setdefault(current_header, TestData(readings))

        file.close()

    def get_test(self, test_type: str) -> TestData:
        return self.get(test_type)


class DataSet(dict):
    """ Class used to hold the tested systems """

    def __init__(self):
        super().__init__()

    def add_data(self, data_name: str, data: ResultSet):
        self.setdefault(data_name, data)

    def get_data(self, data_name: str):
        return self.get(data_name)

    def get_test_stats(self, test_name: str, stat_type: str) -> tuple:
        stat_bundle = ()

        for key, result in self.items():
            try:
                stat_bundle += ((key, result[test_name][stat_type]),)
            except KeyError:
                continue

        return stat_bundle


def get_test_measurement(test_type):
    label = "Unknown measurement"

    if test_type == Test.READ:
        label = "time taken (ns)"

    elif test_type == Test.WRITE:
        label = "time taken (ns)"

    elif test_type == Test.CONST:
        label = "time taken (ns)"

    elif test_type == Test.DEST:
        label = "time taken (ns)"

    elif test_type == Test.IN:
        label = "time taken (ns)"

    elif test_type == Test.OUT:
        label = "time taken (ns)"

    elif test_type == Test.ENC:
        label = "time taken (ns)"

    elif test_type == Test.DEC:
        label = "time taken (ns)"

    return label


def draw_bar_chart(test_type, data: DataSet, colour='g', draw_error=True):
    """ Creates a bar chart displaying a certain test measurement """
    title = test_type + " performance"
    label = get_test_measurement(test_type)

    pyplot.title(title)
    pyplot.ylabel(label)

    # Modifies the retrieved data into matplotlib-friendly formats
    x, y = zip(*data.get_test_stats(test_type, Stat.MEAN))
    _, err = zip(*data.get_test_stats(test_type, Stat.STD_DEV))

    index = numpy.arange(len(x))
    bar_width = 0.4

    if draw_error:
        pyplot.errorbar(index, y, yerr=err, fmt='.', capsize=4)
    pyplot.bar(index, y, color=colour, width=bar_width)
    pyplot.xticks(index, x)
    pyplot.show()


def draw_line_chart(test_type, data: DataSet, colour='b', draw_error=True):
    title = test_type + " performance"
    label = get_test_measurement(test_type)

    pyplot.title(title)
    pyplot.ylabel(label)

    x, y = zip(*data.get_test_stats(test_type, Stat.MEAN))
    _, err = zip(*data.get_test_stats(test_type, Stat.STD_DEV))
    x = list(map(lambda val: int(val.split(' ')[0]), x))

    if draw_error:
        pyplot.errorbar(x, y, yerr=err, fmt='.', capsize=4)

    pyplot.plot(x, y, color=colour)
    pyplot.show()


def sec_to_nsec(sec):
    """ Converts seconds to nanoseconds """
    return sec * 1_000_000_000


def nsec_to_msec(nsec):
    """ Converts nanoseconds to milliseconds """
    return nsec / 1_000_000


def percentage_change(test_one: TestData, test_two: TestData):
    """ Calculates the percentage change between two tests """
    top = test_two[Stat.MEAN] - test_one[Stat.MEAN]
    bottom = test_one[Stat.MEAN]

    return (top / bottom) * 100


def get_data_len(file: str):
    """ Gets the bytes length used in testing """
    with open(file) as file:
        while line := file.readline():
            if Setting.INDICATOR and Setting.DATA_LEN in line:
                return int(line.split()[3])

    return None


def get_data_size(file: str):
    """ Gets the enclave data size used in testing"""
    with open(file) as file:
        while line := file.readline():
            if Setting.INDICATOR and Setting.DATA_LEN in line:
                return int(line.split()[3])

    return None


def get_data_count(file: str):
    """ Gets the number of iterations/repeats per test """
    with open(file) as file:
        while line := file.readline():
            if Setting.INDICATOR and Setting.ITER in line:
                return int(line.split()[3])

    return None


def parse_result_folder(folder_name: str, key_select) -> DataSet:
    """ parses an enclave size result folder into usable result sets """
    files = os.listdir(folder_name)
    files = sorted(files, key=lambda filename: int(filename.split(SEPARATOR)[2]))
    data_set = DataSet()

    for file in files:
        file = folder_name + '/' + file
        data_len = key_select(file)

        if data_len:
            data_set.add_data(str(data_len) + " bytes", ResultSet(file))

    return data_set


if __name__ == '__main__':
    native_size_set = parse_result_folder('recordings/native/data_var', get_data_size)
    native_length_set = parse_result_folder('recordings/native/size_var', get_data_count)

    draw_line_chart(Test.DEC, native_size_set, 'y')
