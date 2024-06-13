#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "Core.h"
#include "Log.h"
#include "Token.h"
#include "Arena.h"
#include "Node.h"

class Parser{
public:
    inline Parser(std::vector<Token> vector)
            : tokens(vector), m_allocator(1024 * 1024 * 10) // 10 MB
    {}
    inline ~Parser(){ m_allocator.Delete(); }
    Node::Program* parse();

private:

    Node::Term* parseTerm();
    std::optional<Node::Stmt*> parseStmt();
    bool isBinOp(const TokenType type);
    int getBinPrec(TokenType type);
    Node::Expr* parseIntExpr(const int min_prec);
    void checkIfLastToken(const char* msg);
    TokenType getNextToken(bool newline = false);

    std::vector<Token> tokens;
    uint64_t index;
    Node::Program program;
    ArenaAllocator m_allocator;
};