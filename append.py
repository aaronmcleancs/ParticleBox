import os

def append_all_files(output_filename):
    with open(output_filename, 'w', encoding='utf-8') as outfile:
        for folder in ["Engine", "UI"]:
            if os.path.isdir(folder):
                for root, dirs, files in os.walk(folder):
                    for filename in files:
                        file_path = os.path.join(root, filename)
                        if os.path.abspath(file_path) == os.path.abspath(output_filename):
                            continue
                        outfile.write(f"--- Start of {file_path} ---\n")
                        try:
                            with open(file_path, 'r', encoding='utf-8') as infile:
                                outfile.write(infile.read())
                        except Exception as e:
                            outfile.write(f"Error reading {file_path}: {e}\n")
                        outfile.write(f"\n--- End of {file_path} ---\n\n")

if __name__ == "__main__":
    output_file = "combined.txt"
    append_all_files(output_file)
    print(f"All files have been appended to {output_file}")