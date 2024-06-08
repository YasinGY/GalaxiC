#include "Generator.h"

void Generator::GenTerm(Node::Term *term) {
    
}

void Generator::GenExpr(Node::Expr *expr) {

}

std::string Generator::Generate() {
    code.data << "section .data\n";
    code.bbs << "section .bbs\n";
    code.text << "section .text\n";

    /// TEMPORARY
    code.text << "global main\n";
    code.text << "main:\n";


    struct ProgVisitor {
        Generator* gen;
        ProgVisitor(Generator* generator) : gen(generator) {}

        void operator()(Node::Exit* stmt){
            switch(gen->target) {
                case PLATFORM_WIN32:
                case PLATFORM_WIN64:
                    gen->GenExpr(stmt->expr);
                    gen->code.text << "ret\n";
                    break;
                case PLATFORM_LINUX32:
                    gen->GenExpr(stmt->expr);
                    gen->code.text << "mov edi, eax\n";
                    gen->code.text << "mov eax, 60\n";
                    gen->code.text << "syscall\n";
                    break;
                case PLATFORM_LINUX64:
                    gen->GenExpr(stmt->expr);
                    gen->code.text << "mov rdi, rax\n";
                    gen->code.text << "mov rax, 60\n";
                    gen->code.text << "syscall\n";
                    break;
            }
        }

        void operator()(Node::Link* stmt){
            gen->prg_links.emplace_back(stmt->value->value);
        }

        void operator()(Node::Variable* stmt){

        }
    };

    for(Node::Stmt stmt : prg->prg) {
        ProgVisitor visitor(this);
        std::visit(visitor, stmt.stmt);
    }

    return code.external.str() + code.data.str() + code.bbs.str() + code.text.str();
}

std::vector<std::string> Generator::GetLinkPrograms() {
    return std::vector<std::string>();
}