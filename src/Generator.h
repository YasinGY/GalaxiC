#pragma once

#include <string>
#include <sstream>
#include <memory>

#include "Core.h"
#include "Node.h"
#include "Log.h"
#include "Storage.h"

class Generator{
public:
    Generator(Node::Program* p, const int t, const int m) : prg(p), target(t), mode(m) {}

    std::string Generate();
    std::vector<std::string> GetLinkPrograms();

private:

    struct AsmStructure{
        std::stringstream external;
        std::stringstream data;
        std::stringstream bbs;
        std::stringstream text;
    };

    void GenTerm(Node::Term* term);
    void GenExpr(Node::Expr* expr); // output will be stored in rax

    Node::Program* prg;
    std::vector<std::string> prg_links;
    AsmStructure code;
    int target;
    int mode;
};