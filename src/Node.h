#pragma once

#include "PCH.h"
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

    enum class LitBool{
        _true, _false
    };

    enum class Comparison{
        None = 0,
        equal, not_equal, greater, greater_equal, less, less_equal
    };

    struct BoolTermInt{
        IntExpr* lhs;
        Comparison comp;
        IntExpr* rhs;
    };

    struct BoolTermParen{
        BoolExpr* expr;
    };

    struct BoolTermBool{
        std::variant<LitBool, Ident*> lhs;
        Comparison comp;
        std::variant<LitBool, Ident*> rhs;
    };

    struct BoolTerm{
        std::variant<BoolTermInt*, BoolTermBool*, BoolTermParen*> term;
    };

    struct BoolExprAnd{
        std::variant<BoolTerm*, BoolExpr*> lhs;
        std::variant<BoolTerm*, BoolExpr*> rhs;
    };

    struct BoolExprOr{
        std::variant<BoolTerm*, BoolExpr*> lhs;
        std::variant<BoolTerm*, BoolExpr*> rhs;
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

    struct Expr{
        std::variant<BoolExpr*, IntExpr*> expr;
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

    struct While{
        BoolExpr* expr;
        std::optional<Scope*> scope;
    };

    enum class Asm_Section{
        text, data, bss, external
    };

    struct Assembly{
        LitString* code;
        Asm_Section section;
    };

    struct Stmt{
        std::variant<Exit*, Link*, Variable*, Scope*, If*, Reassign*, Assembly*, Elif*, Else*, While*> stmt;
    };

    struct Program{
        std::vector<Stmt*> prg;
    };
}