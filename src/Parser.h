#pragma once

#include "PCH.h"
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
    inline void Clear(){ m_allocator.Delete(); }
    Node::Program* parse();

private:

    Node::Term* parseTerm();
    std::optional<Node::Stmt*> parseStmt();
    bool isBinOp(const TokenType type);
    bool isLitBool(TokenType type);
    bool isIntIdent(const std::string& ident);
    int getBinPrec(TokenType type);
    Node::IntExpr* parseIntExpr(const int min_prec = 0);
    VarType getIdentType(const std::string& ident);
    Node::BoolTerm* parseBoolTerm();
    Node::Comparison parseComparison();
    Node::BoolExpr* parseBoolExpr();
    void checkIfLastToken(const std::string& msg, bool add_pos = true);
    std::string getNextTokenPos();
    TokenType getNextToken(bool newline = false);

    std::vector<Token> tokens;
    uint64_t index;
    Node::Program program;
    ArenaAllocator m_allocator;
};