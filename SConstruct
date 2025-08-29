import os
import base64

import time

import urllib.request
import subprocess

def get_resource_files(directory):
    resource_files = []
    for (dirpath, dirnames, filenames) in os.walk("resources"):
        resource_files += [os.path.join(dirpath, file).replace("\\","/") for file in filenames]
    return resource_files

def produce_resource_code(files):
    print("Writing resources...")

    content = """ // GENERATED CODE; DO NOT TOUCH
    #include <map>
    #include <vector>
    #include <string>
    #include <algorithm>
    #include <stdexcept>
    
    namespace resources 
    {
        
    """
    
    for filepath in files:
        with open(filepath, "rb") as file:
            print("Writing " + filepath)
            variable_name = filepath.replace("/", "_").replace("-", "_").replace(".", "_")
            content += "\nconst std::uint8_t " + variable_name + "[] = "
            content += "{" + ', '.join([str(hex(b)) for b in file.read()]) + "}"
            content += ";\n"


    content += """;

    
    std::vector<std::uint8_t> get_file(std::string name)
    {
        std::string sanitized_name = name;
        std::replace(sanitized_name.begin(), sanitized_name.end(), '\\\\', '_');
        std::replace(sanitized_name.begin(), sanitized_name.end(), '/', '_');
        std::replace(sanitized_name.begin(), sanitized_name.end(), '-', '_');
        std::replace(sanitized_name.begin(), sanitized_name.end(), '.', '_');

    """

    for filepath in files:
        variable_name = filepath.replace("/", "_").replace("-", "_").replace(".", "_")
        content += "    if (sanitized_name.compare(\"" + variable_name + "\") == 0) return std::vector<std::uint8_t>(" + variable_name + ", " + variable_name + " + (sizeof(" + variable_name + ") / sizeof(std::uint8_t)));\n"

    content += """  
        throw std::logic_error("couldn't find file " + sanitized_name);
    }
    
    }"""

    with open("src/resources.cpp", "w") as output_file:
        output_file.write(content)

    print("Wrote resources.cpp")

    with open("src/resources.hpp", "w") as output_file:
        output_file.write("""// GENERATED CODE; DO NOT TOUCH

namespace resources 
{
    std::vector<std::uint8_t> get_file(std::string name);
}""")

    print("Wrote resources.hpp")

def set_icon(target, source, env):
    # RCEDIT for setting exe icon
    RCEDIT_URL = "https://github.com/electron/rcedit/releases/download/v1.1.1/rcedit-x86.exe"

    if not os.path.exists(OUTPUT_FOLDER + "/rcedit.exe"):
        print("downloading rcedit...")
        urllib.request.urlretrieve(RCEDIT_URL, OUTPUT_FOLDER + "/rcedit.exe")

    os.chdir(OUTPUT_FOLDER)
    os.system("rcedit.exe " + OUTPUT_NAME + ".exe " + "--set-icon \"../icon.ico\"")


if GetOption("clean") == False:
    files = get_resource_files("html")
    produce_resource_code(files)


OUTPUT_FOLDER = "bin"
OUTPUT_NAME = "CODMVM_LAUNCHER"

# OpenSSL include paths go here
#
# Binaries available at 
# https://slproweb.com/products/Win32OpenSSL.html
OPENSSL_INCLUDE_PATH = "C:/Program Files/OpenSSL-Win32/include"
OPENSSL_LIB_PATH = "C:/Program Files/OpenSSL-Win32/lib/VC/static"
OPENSSL_LIBRARIES = ["libssl32MT", "libcrypto32MT"]

source_files = Glob("src/*.cpp")
source_files.extend(Glob("src/*/*.cpp"))

env = Environment(CCFLAGS=["/std:c++17", "/EHsc"], CPPPATH = [OPENSSL_INCLUDE_PATH], TARGET_ARCH='x86')
program = env.Program(OUTPUT_FOLDER + "/" + OUTPUT_NAME, source_files, LIBS=OPENSSL_LIBRARIES + ["WS2_32", "ADVAPI32", "CRYPT32", "USER32", "SHELL32", "GDI32", "GDIPLUS", "SHLWAPI", "OLE32"], LIBPATH=[OPENSSL_LIB_PATH])

set_icon_command = Command('set_icon', [], set_icon)
Depends(set_icon_command, program)