#pragma once

#include "PCH.h"
#include "Core.h"
#include "Log.h"

class Assemble {
public:
    Assemble(const std::string& src, const std::vector<std::string>& link, const std::string& output,
             int target) :
             links(link), content(src), output_path(output), target(target)
    {
        Store();
        AssembleFile();
        LinkFile();
        Clean();
        Run();
    }

private:

    void Store();
    void AssembleFile();
    void LinkFile();
    void Clean();
    void Run();

    const std::vector<std::string> links;
    const std::string content;
    std::string output_path;
    int target;
};