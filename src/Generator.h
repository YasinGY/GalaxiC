#pragma once

#include <string>
#include <sstream>

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

        code.data << "section .data\n";
        code.bbs << "section .bss\n";
        code.text << "section .text\n";

        /// TEMPORARY
        code.text << "global main\n";
        code.text << "main:\n";
    }

    std::vector<std::string> GetLinkPrograms();
    inline std::string GenerateCode() {
        for(index = 0; index <= prg->prg.size() - 1; index++)
            Generate(prg->prg.at(index));

        return code.external.str() + code.data.str() + code.bbs.str() + code.text.str();
    };

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
    void Generate(const Node::Stmt* stmt);
    inline bool isNextNodeIfChain() {
        if (isLastNode()) {
            return false;
        }
        if (!std::holds_alternative<Node::Elif*>(prg->prg.at(index + 1)->stmt) &&
            !std::holds_alternative<Node::Else*>(prg->prg.at(index + 1)->stmt)) {
            return false;
        }
        return true;
    }
    inline bool isLastNode() {
        return index >= prg->prg.size() - 1;
    }

    Node::Program* prg;
    std::vector<std::string> prg_links;
    AsmStructure code;
    Storage storage;
    std::string bit;
    int target;

    // labels
    uint64_t temp_labels = 0;
    // label called label + the number it is holding like label0
    // used for in a condition chain with else if and else
    uint64_t label_labels = 0;
    uint64_t main_labels = 0;
    uint64_t index;
};