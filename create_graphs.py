#!/usr/bin/env python3
#  -*- coding: UTF-8 -*-

from matplotlib import pyplot
import numpy

OUTPUT_FILENAME = "output.txt"


class Test:
    """ Testing types """
    READ = "Read"
    WRITE = "Write"
    CONST = "Construct"
    DEST = "Destruct"
    IN = "Input"
    OUT = "Output"


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
    HEADER_INDICATOR = "--"

    def __init__(self, filename: str):
        super().__init__()
        self.__construct_result_set(filename)

    def __construct_result_set(self, filename: str):
        """ Constructs/inits the result set """
        current_header = None
        readings = []

        file = open(filename)

        for line in file:
            if ResultSet.HEADER_INDICATOR in line:
                if current_header:
                    self.setdefault(current_header, TestData(readings))
                current_header = line.strip("- \n").split(" ")[0].capitalize()
                readings = []

                continue

            if current_header:
                if line.strip() != "":
                    recording_result = line.split(";")
                    readings.append(sec_to_nsec(int(recording_result[1].strip("\n (ns)")))
                                    + int(recording_result[2].strip("\n (ns)")))

        if current_header and readings != []:
            self.setdefault(current_header, TestData(readings))

        file.close()

    def get_test(self, test_type: str) -> TestData:
        return self.get(test_type)


class SystemSet(dict):
    """ Class used to hold the tested systems """
    def __init__(self):
        super().__init__()

    def add_system(self, system_name: str, data: ResultSet):
        self.setdefault(system_name, data)

    def get_system(self, system_name: str):
        return self.get(system_name)

    def get_test_stats(self, test_name: str, stat_type: str) -> tuple:
        stat_bundle = ()

        for key, result in self.items():
            stat_bundle += (result[test_name][stat_type], )

        return stat_bundle


class Chart:
    """ Class used to draw data charts """
    @staticmethod
    def draw_bar_chart_read(result_set, colour='g'):
        """ Creates a bar chart displaying read measurements """
        pyplot.title("Read performance")
        pyplot.ylabel("time taken (ns)")

        Chart.__finalise_chart(result_set, Test.READ, colour)

    @staticmethod
    def draw_bar_chart_write(result_set, colour='g'):
        """ Creates a bar chart displaying write measurements """
        pyplot.title("write performance")
        pyplot.ylabel("time taken (ns)")

        Chart.__finalise_chart(result_set, Test.WRITE, colour)

    @staticmethod
    def draw_bar_chart_const(result_set, colour='g'):
        """ Creates a bar chart displaying construct measurements """
        pyplot.title("Construct performance")
        pyplot.ylabel("time taken (ns)")

        Chart.__finalise_chart(result_set, Test.CONST, colour)

    @staticmethod
    def draw_bar_chart_dest(result_set, colour='g'):
        """ Creates a bar chart displaying destruct measurements """
        pyplot.title("Destruct performance")
        pyplot.ylabel("time taken (ns)")

        Chart.__finalise_chart(result_set, Test.DEST, colour)

    @staticmethod
    def draw_bar_chart_input(result_set, colour='g'):
        """ Creates a bar chart displaying input measurements """
        pyplot.title("Input performance")
        pyplot.ylabel("time taken (ns)")

        Chart.__finalise_chart(result_set, Test.IN, colour)

    @staticmethod
    def draw_bar_chart_output(result_set: SystemSet, colour='g'):
        """ Creates a bar chart displaying output measurements """
        pyplot.title("Output performance")
        pyplot.ylabel("time taken (ns)")

        Chart.__finalise_chart(result_set, Test.OUT, colour)

    @staticmethod
    def __finalise_chart(data: SystemSet, test_type: str, colour):
        """ Helper function used to finish off the created bar charts """
        systems = tuple(data.keys())
        index = numpy.arange(len(systems))
        bar_width = 0.4

        pyplot.errorbar(index, data.get_test_stats(test_type, Stat.MEAN),
                        yerr=data.get_test_stats(test_type, Stat.STD_DEV), fmt='.', capsize=4)
        pyplot.bar(index, data.get_test_stats(test_type, Stat.MEAN), color=colour, width=bar_width)
        pyplot.xticks(index, systems)
        pyplot.show()


def sec_to_nsec(sec):
    """ Converts seconds to nanoseconds """
    return sec * 1_000_000_000


def nsec_to_msec(nsec):
    """ Converts nanoseconds to milliseconds """
    return nsec / 1_000_000


if __name__ == '__main__':
    system_set = SystemSet()
    system_set.add_system("SGX Driver", ResultSet(OUTPUT_FILENAME))
    system_set.add_system("SGX KVM", ResultSet(OUTPUT_FILENAME))
    system_set.add_system("Virt SGX", ResultSet(OUTPUT_FILENAME))

    Chart.draw_bar_chart_input(system_set, 'b')
    Chart.draw_bar_chart_output(system_set, 'y')
