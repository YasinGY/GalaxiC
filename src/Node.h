#pragma once

#include <string>
#include <variant>
#include <vector>

#include "Variable.h"
#include "Token.h"

namespace Node{
    enum class BinOp {
        none = 0, add, sub, mul, div, rest
    };

    struct Expr;

    struct LitString{
        std::string value;
    };

    struct LitInt{
        std::string value;
    };

    struct Ident{
        std::string value;
    };

    struct TermParen{
        Expr* expr;
    };

    struct Term{
        std::variant<LitInt*, Ident*, TermParen*> term;
    };

    struct BinExprAdd{
        Expr* lhs;
        Expr* rhs;
    };

    struct BinExprSub{
        Expr* lhs;
        Expr* rhs;
    };

    struct BinExprMul{
        Expr* lhs;
        Expr* rhs;
    };

    struct BinExprDiv{
        Expr* lhs;
        Expr* rhs;
    };

    struct BinExprMod{
        Expr* lhs;
        Expr* rhs;
    };

    struct BinExpr{
        std::variant<BinExprAdd*, BinExprSub*, BinExprMul*, BinExprDiv*, BinExprMod*> expr;
    };

    struct Expr{
        std::variant<Term*, BinExpr*> var;
    };

    struct Exit{
        Expr* expr;
    };

    struct Variable{
        VarType type;
        Expr* expr;
        Ident* ident;
    };

    struct Link{
        LitString* value;
    };

    struct Stmt;

    struct Scope{
        std::vector<Stmt> stmts;
    };

    struct Stmt{
        std::variant<Exit*, Link*, Variable*, Scope*> stmt;
    };

    struct Program{
        std::vector<Stmt> prg;
    };
}