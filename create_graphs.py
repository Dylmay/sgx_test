import json
from matplotlib import pyplot
import numpy

OUTPUT_FILENAME = "output.txt"
HEADER_INDICATOR = "--"


def to_dict(file) -> dict:
    output_dict = {}
    current_header = None

    for line in file:
        if HEADER_INDICATOR in line:
            current_header = line.strip("- \n")
            output_dict.setdefault(current_header, [])
            continue

        if current_header:
            if line.strip() != "":
                recording_result = line.split(";")[1].strip("\n ns")
                output_dict[current_header].append(int(recording_result))

    return output_dict


def average(dct: dict):
    tuple = ()

    for value in dct.values():
        tuple += (sum(value) / len(value), )

    return tuple


def create_bar_chart_read(data):
    pyplot.title("Read performance")
    pyplot.ylabel("time taken (ns)")

    __finalise_chart(data)


def create_bar_chart_write(data):
    pyplot.title("write performance")
    pyplot.ylabel("time taken (ns)")

    __finalise_chart(data)


def create_bar_chart_enc(data):
    pyplot.title("Construct/Destruct performance")
    pyplot.ylabel("time taken (ns)")

    __finalise_chart(data)


def __finalise_chart(data):
    systems = 'SGX Driver', 'SGX KVM', 'Virtualized SGX'
    index = numpy.arange(len(systems))
    bar_width = 0.4

    pyplot.bar(index, data, color='g', width=bar_width)
    pyplot.xticks(index, systems)
    pyplot.show()


def bundle_data(sgx_driver, sgx_kvm, virt_sgx):
    return [
            (sgx_driver[0], sgx_kvm[0], virt_sgx[0]), (sgx_driver[1], sgx_kvm[1], virt_sgx[1]),
            (sgx_driver[2], sgx_kvm[2], virt_sgx[2])
    ]


if __name__ == '__main__':
    output_file = open(OUTPUT_FILENAME)
    result_dict = to_dict(output_file)

    print(json.dumps(result_dict, indent=2))
    print(average(result_dict))

    data = bundle_data(average(result_dict), average(result_dict), average(result_dict))

    create_bar_chart_write(data[0])
    create_bar_chart_read(data[1])
    create_bar_chart_enc(data[2])
