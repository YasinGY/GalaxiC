#include "Generator.h"

void Generator::GenTerm(const Node::Term *term, const std::string reg) {
    if(std::holds_alternative<Node::LitInt*>(term->term)){
        std::string value = std::get<Node::LitInt*>(term->term)->value;

        switch(target){
            case PLATFORM_LINUX32:
            case PLATFORM_WIN32:
                code.text << "mov " << reg <<  ", " << value << "\n";
                break;
            case PLATFORM_WIN64:
            case PLATFORM_LINUX64:
                code.text << "mov " << reg << ", " << value << "\n";
                break;
        }
    }
    else if(std::holds_alternative<Node::Ident*>(term->term)){
        auto ident = std::get<Node::Ident*>(term->term);

        if(!storage.IdentExists(ident->value)){
            Log::Error("Unknown ident `" + ident->value + "` in the int expression");
            exit(1);
        }

        if(!storage.IsIdentInit(ident->value)){
            Log::Error("Ident `" + ident->value + "` was used because the value was initialized");
            exit(1);
        }

        code.text << "mov " << bit <<
        "ax, [" << bit << "sp + " << storage.GetStackPosition(ident->value) << "]\n";
    }
}

void Generator::GenBinExpr(const Node::BinExpr* expr) {
    struct BinExprVisitor{
        Generator& gen;
        BinExprVisitor(Generator& generator) : gen(generator) {}

        void operator()(const Node::BinExprMul* expr){
            gen.GenExpr(expr->lhs, gen.bit + "ax");
            gen.GenExpr(expr->rhs, gen.bit + "cx");
            gen.code.text << "mul " << gen.bit << "cx\n";
        }

        void operator()(const Node::BinExprDiv* expr){
            gen.GenExpr(expr->lhs, gen.bit + "ax");
            gen.code.text << "xor " << gen.bit << "dx, " << gen.bit << "dx\n";
            gen.GenExpr(expr->rhs, gen.bit + "cx");
            gen.code.text << "div " << gen.bit << "cx\n";
        }

        void operator()(const Node::BinExprAdd* expr){
            gen.GenExpr(expr->lhs, gen.bit + "ax");
            gen.code.text << "mov " << gen.bit << "bx, " << gen.bit << "ax\n";
            gen.GenExpr(expr->rhs, gen.bit + "ax");
            gen.code.text << "add " << gen.bit << "ax, " << gen.bit << "bx\n";
        }

        void operator()(const Node::BinExprSub* expr){
            gen.GenExpr(expr->lhs, gen.bit + "ax");
            gen.code.text << "mov " << gen.bit << "bx, " << gen.bit << "ax\n";
            gen.GenExpr(expr->rhs, gen.bit + "ax");
            gen.code.text << "sub " << gen.bit << "bx, " << gen.bit << "ax\n";
            gen.code.text << "mov " << gen.bit << "ax, " << gen.bit << "bx\n";
        }

        void operator()(const Node::BinExprMod* expr){
            gen.GenExpr(expr->lhs, gen.bit + "ax");
            gen.code.text << "xor " << gen.bit << "dx, " << gen.bit << "dx\n";
            gen.GenExpr(expr->rhs, gen.bit + "cx");
            gen.code.text << "div " << gen.bit << "cx\n";
            gen.code.text << "mov " << gen.bit << "ax, " << gen.bit << "dx\n";
        }
    };

    BinExprVisitor visitor(*this);
    std::visit(visitor, expr->expr);
}

void Generator::GenExpr(const Node::Expr* expr, const std::string reg) {
    if(std::holds_alternative<Node::Term*>(expr->var))
        GenTerm(std::get<Node::Term*>(expr->var), reg);

    else if(std::holds_alternative<Node::BinExpr*>(expr->var))
        GenBinExpr(std::get<Node::BinExpr*>(expr->var));
}

std::string Generator::Generate() {
    code.data << "section .data\n";
    code.bbs << "section .bss\n";
    code.text << "section .text\n";

    /// TEMPORARY
    code.text << "global main\n";
    code.text << "main:\n";

    struct ProgVisitor {
        Generator& gen;
        ProgVisitor(Generator& generator) : gen(generator) {}

        void operator()(Node::Exit* stmt){
            gen.GenExpr(stmt->expr, gen.bit + "ax");
            if(gen.storage.GetStackSize() > 0){
                gen.code.text << "add " << gen.bit << "sp, " << gen.storage.GetStackSize() << '\n';
            }

            switch(gen.target) {
                case PLATFORM_WIN32:
                case PLATFORM_WIN64:
                    gen.code.text << "ret\n";
                    break;
                case PLATFORM_LINUX32:
                    gen.code.text << "mov " << gen.bit << "di, " << gen.bit << "ax\n";
                    gen.code.text << "mov " <<  gen.bit << "ax, 60\n";
                    gen.code.text << "syscall\n";
                    break;
            }
        }

        void operator()(Node::Link* stmt){
            gen.prg_links.emplace_back(stmt->value->value);
        }

        void operator()(Node::Variable* stmt){
            gen.storage.StoreVariable(stmt->ident->value, gen.isExprInit(stmt->expr), stmt->type);

            switch(stmt->type){
                case VarType::_char:
                    gen.GenExpr(stmt->expr, "al");
                    gen.code.text << "sub " << gen.bit << "sp, 8\n";
                    gen.code.text << "mov [" << gen.bit << "sp], al\n";
                    break;
                case VarType::_int16:
                    gen.GenExpr(stmt->expr, "eax");
                    gen.code.text << "sub " << gen.bit << "sp, 16\n";
                    gen.code.text << "mov [" << gen.bit << "sp], ax\n";
                    break;
                case VarType::_int32:
                case VarType::_float:
                    gen.GenExpr(stmt->expr, "eax");
                    gen.code.text << "sub " << gen.bit << "sp, 32\n";
                    gen.code.text << "mov [" << gen.bit << "sp], eax\n";
                    break;
                case VarType::_int64:
                case VarType::_double:
                    if(gen.target == PLATFORM_LINUX32 || gen.target == PLATFORM_WIN32){
                        Log::Error("You cant have an long/int64 in a 32-bit program");
                        exit(1);
                    }
                    gen.GenExpr(stmt->expr, "rax");
                    gen.code.text << "sub " << gen.bit << "sp, 64\n";
                    gen.code.text << "mov [" << gen.bit << "sp], rax\n";
                    break;
                case VarType::_string:
                    Log::Error("string not implemented");
                    exit(1);
            }
        }
    };

    for(Node::Stmt stmt : prg->prg) {
        ProgVisitor visitor(*this);
        std::visit(visitor, stmt.stmt);
    }

    return code.external.str() + code.data.str() + code.bbs.str() + code.text.str();
}

std::vector<std::string> Generator::GetLinkPrograms() {
    return std::vector<std::string>();
}

bool Generator::isExprInit(const Node::Expr *expr) {
    if(!std::holds_alternative<Node::Term*>(expr->var))
        return true;

    auto term = std::get<Node::Term*>(expr->var);
    if(!std::holds_alternative<Node::Ident*>(term->term))
        return true;

    auto ident = std::get<Node::Ident*>(term->term);
    if(ident->value != "")
        return true;

    return false;
}