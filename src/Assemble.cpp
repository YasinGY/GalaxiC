#include "Assemble.h"

void Assemble::Store() {
    std::ofstream file(output_path.substr(0, output_path.length() - 4) + ".asm");
    file << content;
    file.close();
}

void Assemble::AssembleFile() {
    system(std::string("nasm -f " +
        std::string((target == PLATFORM_WIN32) ? "win32 " : "win64 ") +
        output_path.substr(0, output_path.length() - 4) + ".asm -o " +
        output_path.substr(0, output_path.length() - 4) + ".o"
    ).c_str());
}

void Assemble::LinkFile() {

    std::string command;
    command = "gcc " + output_path.substr(0, output_path.length() - 4) + ".o -o " + output_path;
    for(std::string str : links)
        command += " -l" + str;

    system(command.c_str());
}

void Assemble::Clean() {
    system(std::string("del " + output_path.substr(0, output_path.length() - 4) + ".o")
    .c_str());
}

void Assemble::Run(){
    system(std::string(output_path).c_str());
}