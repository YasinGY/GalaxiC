import os

def count_lines_in_file(filepath):
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as file:
        lines = file.readlines()
        return len(lines)

def main():
    folder_path = 'src'  # Change this if your folder is named differently
    total_lines = 0

    if not os.path.exists(folder_path):
        print(f"The folder '{folder_path}' does not exist.")
        return

    for root, _, files in os.walk(folder_path):
        for file in files:
            filepath = os.path.join(root, file)
            if os.path.isfile(filepath):
                line_count = count_lines_in_file(filepath)
                total_lines += line_count
                print(f"{file}: {line_count} lines")

    print(f"\nTotal lines in the folder '{folder_path}': {total_lines} lines")

if __name__ == "__main__":
    main()
