import json

OUTPUT_FILENAME = "output.txt"
HEADER_INDICATOR = "--"


def to_dict(file) -> dict:
    output_dict = {}
    current_header = None

    for line in output_file:
        if HEADER_INDICATOR in line:
            current_header = line.strip("- \n")
            output_dict.setdefault(current_header, [])
            continue

        if current_header:
            if line.strip() != "":
                recording_result = line.split(";")[1].split()[0]
                output_dict[current_header].append(recording_result)

    return output_dict


if __name__ == '__main__':
    output_file = open(OUTPUT_FILENAME)
    print(json.dumps(to_dict(output_file), indent=2))
