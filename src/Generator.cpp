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

        switch(target){
            case PLATFORM_WIN32:
            case PLATFORM_LINUX32:
                code.text << "mov eax, [esp + " << storage.GetStackPosition(ident->value) << "]\n";
                break;
            case PLATFORM_WIN64:
            case PLATFORM_LINUX64:
                code.text << "mov rax, [rsp + " << storage.GetStackPosition(ident->value) << "]\n";
                break;
        }
    }
}

void Generator::GenBinExpr(const Node::BinExpr* expr) {
    struct BinExprVisitor{
        Generator& gen;
        BinExprVisitor(Generator& generator) : gen(generator) {}

        void operator()(const Node::BinExprMul* expr){
            gen.GenExpr(expr->lhs, "rax");
            gen.GenExpr(expr->rhs, "rcx");
            switch(gen.target) {
                case PLATFORM_WIN32:
                case PLATFORM_LINUX32:
                    gen.code.text << "mul ecx\n";
                    break;
                case PLATFORM_LINUX64:
                case PLATFORM_WIN64:
                    gen.code.text << "mul rcx\n";
                    break;
            }
        }

        void operator()(const Node::BinExprDiv* expr){
            gen.GenExpr(expr->lhs, "rax");
            switch(gen.target) {
                case PLATFORM_WIN32:
                case PLATFORM_LINUX32:
                    gen.code.text << "xor edx, edx\n";
                    break;
                case PLATFORM_LINUX64:
                case PLATFORM_WIN64:
                    gen.code.text << "xor rdx, rdx\n";
                    break;
            }
            gen.GenExpr(expr->rhs, "rcx");
            switch(gen.target) {
                case PLATFORM_WIN32:
                case PLATFORM_LINUX32:
                    gen.code.text << "div ecx\n";
                    break;
                case PLATFORM_LINUX64:
                case PLATFORM_WIN64:
                    gen.code.text << "div rcx\n";
                    break;
            }
        }

        void operator()(const Node::BinExprAdd* expr){
            gen.GenExpr(expr->lhs, "rax");
            switch(gen.target) {
                case PLATFORM_WIN32:
                case PLATFORM_LINUX32:
                    gen.code.text << "mov ebx, eax\n";
                    break;
                case PLATFORM_LINUX64:
                case PLATFORM_WIN64:
                    gen.code.text << "mov rbx, rax\n";
                    break;
            }
            gen.GenExpr(expr->rhs, "rax");
            switch(gen.target) {
                case PLATFORM_WIN32:
                case PLATFORM_LINUX32:
                    gen.code.text << "add eax, ebx\n";
                    break;
                case PLATFORM_LINUX64:
                case PLATFORM_WIN64:
                    gen.code.text << "add rax, rbx\n";
                    break;
            }
        }

        void operator()(const Node::BinExprSub* expr){
            gen.GenExpr(expr->lhs, "rax");
            switch(gen.target) {
                case PLATFORM_WIN32:
                case PLATFORM_LINUX32:
                    gen.code.text << "mov ebx, eax\n";
                    break;
                case PLATFORM_LINUX64:
                case PLATFORM_WIN64:
                    gen.code.text << "mov rbx, rax\n";
                    break;
            }
            gen.GenExpr(expr->rhs, "rax");
            switch(gen.target) {
                case PLATFORM_WIN32:
                case PLATFORM_LINUX32:
                    gen.code.text << "sub ebx, eax\n";
                    gen.code.text << "mov eax, ebx\n";
                    break;
                case PLATFORM_LINUX64:
                case PLATFORM_WIN64:
                    gen.code.text << "sub rbx, rax\n";
                    gen.code.text << "mov rax, rbx\n";
                    break;
            }
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
    code.bbs << "section .bbs\n";
    code.text << "section .text\n";

    /// TEMPORARY
    code.text << "global main\n";
    code.text << "main:\n";

    struct ProgVisitor {
        Generator& gen;
        ProgVisitor(Generator& generator) : gen(generator) {}

        void operator()(Node::Exit* stmt){
            switch(gen.target) {
                case PLATFORM_WIN32:
                    gen.GenExpr(stmt->expr, "eax");
                    gen.code.text << "ret\n";
                    break;
                case PLATFORM_WIN64:
                    gen.GenExpr(stmt->expr, "rax");
                    gen.code.text << "ret\n";
                    break;
                case PLATFORM_LINUX32:
                    gen.GenExpr(stmt->expr, "eax");
                    gen.code.text << "mov edi, eax\n";
                    gen.code.text << "mov eax, 60\n";
                    gen.code.text << "syscall\n";
                    break;
                case PLATFORM_LINUX64:
                    gen.GenExpr(stmt->expr, "rax");
                    gen.code.text << "mov rdi, rax\n";
                    gen.code.text << "mov rax, 60\n";
                    gen.code.text << "syscall\n";
                    break;
            }
        }

        void operator()(Node::Link* stmt){
            gen.prg_links.emplace_back(stmt->value->value);
        }

        void operator()(Node::Variable* stmt){
            assert(false);
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