#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "Core.h"
#include "Node.h"
#include "Log.h"
#include "Tokenizer.h"
#include "Token.h"
#include "Parser.h"
#include "Generator.h"
#include "Assemble.h"

struct Arguments{
    int target;
    std::string input_file;
    std::string output_file;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-compare"
Arguments parseProgramArguments(int argc, char* argv[]){
    Arguments temp;
    if(argc < 2){
        Log::Error("Target input file was not specified");
        exit(1);
    }

    temp.input_file = argv[1];
    unsigned int len = temp.input_file.length();
    if(temp.input_file.substr(len - 3, len) != ".gx"){
        Log::Error("Input program extension must be .gx");
        exit(1);
    }

    for(int i = 2; i + 1 <= argc; i++){
        if(std::string(argv[i]) == "-o"){
            i++;
            temp.output_file = std::string(argv[i]);
        }
        else if(std::string(argv[i]) == "-p"){
            i++;
            if(std::string(argv[i]) == "win32"){
                temp.target = PLATFORM_WIN32;
            }
            else if(std::string(argv[i]) == "win64"){
                temp.target = PLATFORM_WIN64;
            }
            else if(std::string(argv[i]) == "linux32"){
                temp.target = PLATFORM_LINUX32;
            }
            else if(std::string(argv[i]) == "linux64"){
                temp.target = PLATFORM_LINUX64;
            }
            else{
                Log::Error("Unknown target platform as program argument, expected "
                           "something like `win32` with the os name + the architect");
                exit(1);
            }
        }
        else{
            std::string s = std::string(argv[i]);
            Log::Error("Unknown program argument flag name `" + s + "`");
            exit(1);
        }
    }

    return temp;
}
#pragma clang diagnostic pop

std::string readFile(std::string name){
    std::stringstream ret_value;

    std::ifstream file(name);
    if(file.is_open()){
        ret_value << file.rdbuf();
        return ret_value.str();
    }
    Log::Error("Failed to open the given input file \'" + name + "\'.");
    exit(1);
}

// Gxcompier test.gx -p win64 -o test.exe
int main(int argc, char* argv[]){
    Arguments args = parseProgramArguments(argc, argv);

    std::string content = readFile(args.input_file);
    content += ' ';

    std::vector<Token> tokens;

    std::vector<std::string> links;

    {
        Tokenizer tokenizer(content);
        tokens = tokenizer.tokenize();
    }
    {
        Parser parser(tokens);
        Node::Program* prg = parser.parse();
        Generator generator(prg, args.target);
        content = generator.Generate();
        links = generator.GetLinkPrograms();
    }

    Assemble assemble(content, links, args.output_file, args.target);

    return 0;
}