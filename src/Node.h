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

    struct IntExpr;

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
        IntExpr* expr;
    };

    struct Term{
        std::variant<LitInt*, Ident*, TermParen*> term;
    };

    struct BinExprAdd{
        IntExpr* lhs;
        IntExpr* rhs;
    };

    struct BinExprSub{
        IntExpr* lhs;
        IntExpr* rhs;
    };

    struct BinExprMul{
        IntExpr* lhs;
        IntExpr* rhs;
    };

    struct BinExprDiv{
        IntExpr* lhs;
        IntExpr* rhs;
    };

    struct BinExprMod{
        IntExpr* lhs;
        IntExpr* rhs;
    };

    struct BinExpr{
        std::variant<BinExprAdd*, BinExprSub*, BinExprMul*, BinExprDiv*, BinExprMod*> expr;
    };

    struct IntExpr{
        std::variant<Term*, BinExpr*> var;
    };

    struct BoolExpr;

    struct BoolExprAnd{
        BoolExpr* lhs;
        BoolExpr* rhs;
    };

    struct BoolExprOr{
        BoolExpr* lhs;
        BoolExpr* rhs;
    };

    enum class Comparison{
        equal, greater, less, greater_equal, less_equal, not_equal
    };

    struct BoolTermInt{
        IntExpr* lhs;
        Comparison comp;
        IntExpr* rhs;
    };

    struct BoolTermParen{
        BoolExpr* expr;
    };

    struct BoolTerm{
        std::variant<BoolTermInt*, BoolTermParen*> term;
    };

    struct BoolExpr{
        std::variant<BoolExprAnd*, BoolExprOr*, BoolTerm*> expr;
    };

    struct Reassign{
        Ident* ident;
        IntExpr* expr;
    };

    struct Exit{
        IntExpr* expr;
    };

    struct Variable{
        VarType type;
        IntExpr* expr;
        Ident* ident;
    };

    struct Link{
        LitString* value;
    };

    struct Stmt;

    struct Scope{
        std::vector<Stmt*> stmts;
    };

    struct If{
        BoolExpr* expr;
        Scope* stmt;
    };

    struct Elif{
        BoolExpr* expr;
        Scope* stmt;
    };

    struct Else{
        Scope* stmt;
    };

    enum class Asm_Section{
        text, data, bss, external
    };

    struct Assembly{
        LitString* code;
        Asm_Section section;
    };

    struct Stmt{
        std::variant<Exit*, Link*, Variable*, Scope*, If*, Reassign*, Assembly*, Elif*, Else*> stmt;
    };

    struct Program{
        std::vector<Stmt*> prg;
    };
}