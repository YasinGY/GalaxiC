#pragma once

#include <string>
#include <variant>
#include <vector>

#include "Storage.h"
#include "Token.h"

namespace Node{
    enum class BinOp {
        none = 0, add, sub, mul, div, rest
    };

    struct LitInt{
        std::string value;
    };

    struct Ident{
        std::string value;
    };

    struct LitString{
        std::string value;
    };

    struct Term{
        std::variant<LitInt*, Ident*> term;
    };

    struct Expr;

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

    struct BinExpr{
        std::variant<BinExprAdd*, BinExprSub*, BinExprMul*, BinExprDiv*> expr;
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
    };

    struct Link{
        LitString* value;
    };

    struct Stmt{
        std::variant<Exit*, Link*, Variable*> stmt;
    };

    struct Program{
        std::vector<Stmt> prg;
    };
}