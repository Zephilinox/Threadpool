import json
import sys

def main():
    COMPILE_COMMANDS_INPUT_PATH = sys.argv[1]
    COMPILE_COMMANDS_OUTPUT_PATH = sys.argv[2]
    FILTER_FILES_CONTAINING = sys.argv[3]

    print(f"\nLoading from {COMPILE_COMMANDS_INPUT_PATH}")
    with open(COMPILE_COMMANDS_INPUT_PATH, "r") as read_file:
        data_to_filter = json.load(read_file)

    data_to_keep = []
    for section in data_to_filter:
        should_filter = False
        for key, value in section.items():
            if (key == "file" or key == "directory"):
                should_filter = FILTER_FILES_CONTAINING in value
                if should_filter:
                    print(f"Filtering {value}")
        if not should_filter:
            data_to_keep.append(section)

    print(f"Saving to {COMPILE_COMMANDS_OUTPUT_PATH}\n")
    with open(COMPILE_COMMANDS_OUTPUT_PATH, "w") as write_file:
        json.dump(data_to_keep, write_file, indent=4)

if __name__ == '__main__':
    main()
