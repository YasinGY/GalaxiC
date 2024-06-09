#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <cassert>

#include "Core.h"
#include "Node.h"
#include "Log.h"
#include "Storage.h"

class Generator{
public:
    Generator(Node::Program* p, const int t) : prg(p), target(t) {}

    std::string Generate();
    std::vector<std::string> GetLinkPrograms();

private:

    struct AsmStructure{
        std::stringstream external;
        std::stringstream data;
        std::stringstream bbs;
        std::stringstream text;
    };

    void GenTerm(const Node::Term* term, const std::string reg);
    void GenExpr(const Node::Expr* expr, const std::string reg);
    void GenBinExpr(const Node::BinExpr* expr);

    Node::Program* prg;
    std::vector<std::string> prg_links;
    AsmStructure code;
    Storage storage;
    int target;
};