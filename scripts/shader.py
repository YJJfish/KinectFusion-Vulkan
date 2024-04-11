import argparse
import binascii
import pathlib

parser = argparse.ArgumentParser(
    prog="shader.py",
    description="Python script that converts spv files to C string literals",
    allow_abbrev=True
)
parser.add_argument("input", help="path to input spv file")
parser.add_argument("-o", "--output", nargs="?", help="path to output C header file. If not given, the program uses the input file path followed by '.h' as the output file path.")
parser.add_argument("-v", "--var", nargs="?", help="the variable name of the C string literal. If not given, the program uses the input file name with '.' replaced by '_' as the variable name.")
args = parser.parse_args()
input_path = args.input
output_path = args.output
if output_path is None:
    output_path = input_path + ".h"
var_name = args.var
if var_name is None:
    var_name = pathlib.Path(input_path).name.replace('.', '_')

try:
    fin = open(input_path, "rb")
    spv = fin.read()
    fin.close()
except (OSError):
    print("Error opening file {}".format(input_path))
    exit(-1)

try:
    fout = open(output_path, "w")
    hex = binascii.hexlify(spv).decode("UTF-8")
    hex_string = ", ".join(["'\\x" + hex[i:i+2] + "'" for i in range(0, len(hex), 2)])
    fout.write("constexpr char {}[] = {{ {} }};\n".format(var_name, hex_string))
    fout.close()
except (OSError):
    print("Error opening file {}".format(output_path))
    exit(-1)
