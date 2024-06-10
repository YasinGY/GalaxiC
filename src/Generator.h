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
    inline Generator(Node::Program* p, const int t) : prg(p), target(t) {
        switch(target){
            case PLATFORM_LINUX32:
            case PLATFORM_WIN32:
                bit = 'e';
                break;
            case PLATFORM_WIN64:
            case PLATFORM_LINUX64:
                bit = 'r';
                break;
        }
    }

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
    bool isExprInit(const Node::Expr* expr);

    Node::Program* prg;
    std::vector<std::string> prg_links;
    AsmStructure code;
    Storage storage;
    std::string bit;
    int target;
};