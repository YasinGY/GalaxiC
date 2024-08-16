#include "Generator.h"

void Generator::GenTerm(const Node::Term *term, const std::string reg) {
    if(std::holds_alternative<Node::LitInt*>(term->term)){
        std::string value = std::get<Node::LitInt*>(term->term)->value;

        code.text << "mov " << reg << ", " << value << '\n';
    }
    else if(std::holds_alternative<Node::Ident*>(term->term)){
        auto ident = std::get<Node::Ident*>(term->term);

        if(!storage.IsIdentInit(ident->value)){
            Log::Error("Ident `" + ident->value + "` was used because the value was initialized");
            exit(1);
        }

        code.text << "mov " << reg << ", [" << bit << "sp + " << storage.GetStackPosition(ident->value) << "]\n";
    }
    else if(std::holds_alternative<Node::TermParen*>(term->term)){
        auto paren = std::get<Node::TermParen*>(term->term);
        GenExpr(paren->expr, reg);
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

void Generator::GenExpr(const Node::IntExpr* expr, const std::string reg) {
    if(std::holds_alternative<Node::Term*>(expr->var))
        GenTerm(std::get<Node::Term*>(expr->var), reg);

    else if(std::holds_alternative<Node::BinExpr*>(expr->var)) {
        GenBinExpr(std::get<Node::BinExpr *>(expr->var));
        code.text << "mov " << reg << ", " << bit << "ax\n";
    }
}

void Generator::GenBoolTerm(const Node::BoolTerm *term, const std::string reg) {
    if(std::holds_alternative<Node::BoolTermInt*>(term->term)){
        auto int_term = std::get<Node::BoolTermInt*>(term->term);

        GenExpr(int_term->lhs, bit + "10");
        GenExpr(int_term->rhs, bit + "11");

        code.text << "mov " << bit << "ax, 0\n";
        code.text << "cmp r10, r11\n";

        switch(int_term->comp){
            case Node::Comparison::equal:
                code.text << "je " << labels.getBoolLabel(false) << '\n';
                code.text << "jmp " << labels.getCurrentLabel(false) << '\n';
                code.text << labels.getBoolLabel() << ":\n";
                code.text << "mov " << bit << "ax, 1\n";
                labels.addBoolLabel();
                break;
            case Node::Comparison::not_equal:
                code.text << "jne " << labels.getBoolLabel(false) << '\n';
                code.text << "jmp " << labels.getCurrentLabel(false) << '\n';
                code.text << labels.getBoolLabel() << ":\n";
                code.text << "mov " << bit << "ax, 1\n";
                labels.addBoolLabel();
                break;
            case Node::Comparison::greater:
                code.text << "jg " << labels.getBoolLabel(false) << '\n';
                code.text << "jmp " << labels.getCurrentLabel(false) << '\n';
                code.text << labels.getBoolLabel() << ":\n";
                code.text << "mov " << bit << "ax, 1\n";
                labels.addBoolLabel();
                break;
            case Node::Comparison::greater_equal:
                code.text << "jg " << labels.getBoolLabel(false) << '\n';
                code.text << "je " << labels.getBoolLabel(false) << '\n';
                code.text << "jmp " << labels.getCurrentLabel(false) << '\n';
                code.text << labels.getBoolLabel() << ":\n";
                code.text << "mov " << bit << "ax, 1\n";
                labels.addBoolLabel();
                break;
            case Node::Comparison::less:
                code.text << "jl " << labels.getBoolLabel(false) << '\n';
                code.text << "jmp " << labels.getCurrentLabel(false) << '\n';
                code.text << labels.getBoolLabel() << ":\n";
                code.text << "mov " << bit << "ax, 1\n";
                labels.addBoolLabel();
                break;
            case Node::Comparison::less_equal:
                code.text << "jl " << labels.getBoolLabel(false) << '\n';
                code.text << "je " << labels.getBoolLabel(false) << '\n';
                code.text << "jmp " << labels.getCurrentLabel(false) << '\n';
                code.text << labels.getBoolLabel() << ":\n";
                code.text << "mov " << bit << "ax, 1\n";
                labels.addBoolLabel();
                break;
        }
    }

    else if(std::holds_alternative<Node::BoolTermBool*>(term->term)){
        auto bool_term = std::get<Node::BoolTermBool*>(term->term);

        // Left side
        if(std::holds_alternative<Node::LitBool>(bool_term->lhs)){
            if(std::get<Node::LitBool>(bool_term->lhs) == Node::LitBool::_true)
                code.text << "mov " << bit << "10, 1\n";
            else
                code.text << "mov " << bit << "10, 0\n";
        }
        else{
            std::string ident = std::get<Node::Ident*>(bool_term->lhs)->value;

            if(!storage.IsIdentInit(ident)){
                Log::Error("Ident `" + ident + "` was never initialized");
                exit(1);
            }

            code.text << "mov " << bit << "10, [" << bit << "sp + " << storage.GetStackPosition(ident) << "]\n";
        }

        // Right side
        if(std::holds_alternative<Node::LitBool>(bool_term->rhs)){
            if(std::get<Node::LitBool>(bool_term->rhs) == Node::LitBool::_true)
                code.text << "mov " << bit << "11, 1\n";
            else
                code.text << "mov " << bit << "11, 0\n";
        }
        else{
            std::string ident = std::get<Node::Ident*>(bool_term->rhs)->value;

            if(!storage.IsIdentInit(ident)){
                Log::Error("Ident `" + ident + "` was never initialized");
                exit(1);
            }

            code.text << "mov " << bit << "11, [" << bit << "sp + " << storage.GetStackPosition(ident) << "]\n";
        }

        code.text << "mov " << bit << "ax, 1\n";
        code.text << "cmp r10, r11\n";
        switch(bool_term->comp) {
            case Node::Comparison::equal:
                code.text << "je " << labels.getBoolLabel(false) << '\n';
                code.text << "jmp " << labels.getCurrentLabel(false) << '\n';
                code.text << labels.getBoolLabel() << ":\n";
                code.text << "mov " << bit << "ax, 1\n";
                labels.addBoolLabel();
                break;
            case Node::Comparison::not_equal:
                code.text << "jne " << labels.getBoolLabel(false) << '\n';
                code.text << "jmp " << labels.getCurrentLabel(false) << '\n';
                code.text << labels.getBoolLabel() << ":\n";
                code.text << "mov " << bit << "ax, 1\n";
                labels.addBoolLabel();
                break;
        }
    }

    else{
        GenBoolExpr(std::get<Node::BoolTermParen*>(term->term)->expr, reg);
        return;
    }

    code.text << labels.getLastLabel() << ":\n";
    labels.addCurrentLabel();

    if(reg != bit + "ax")
        code.text << "mov " << reg << ", " << bit << "ax\n";
}

/// Puts 0 in the argument `reg` if false and 1 if its true
void Generator::GenBoolExpr(const Node::BoolExpr *expr, const std::string reg, const bool continue_last_label) {
    struct BoolExprVisitor{
        Generator& gen;
        BoolExprVisitor(Generator& generator) : gen(generator) {}

        void operator()(const Node::BoolTerm* term){
            gen.GenBoolTerm(term, gen.bit + "ax");
        }

        void operator()(const Node::BoolExprAnd* expr){
            if(std::holds_alternative<Node::BoolTerm*>(expr->lhs))
                gen.GenBoolTerm(std::get<Node::BoolTerm*>(expr->lhs), gen.bit + "8");
            else
                gen.GenBoolExpr(std::get<Node::BoolExpr*>(expr->lhs), gen.bit + "8");

            if(std::holds_alternative<Node::BoolTerm*>(expr->rhs))
                gen.GenBoolTerm(std::get<Node::BoolTerm*>(expr->rhs), gen.bit + "9");
            else
                gen.GenBoolExpr(std::get<Node::BoolExpr*>(expr->rhs), gen.bit + "9");


            gen.code.text << "cmp " << gen.bit << "8, 0" << '\n';
            gen.code.text << "je " << gen.labels.getCurrentLabel(false) << '\n';
            gen.code.text << "cmp " << gen.bit << "9, 0" << '\n';
            gen.code.text << "je " << gen.labels.getCurrentLabel(false) << '\n';
            gen.code.text << "jmp " << gen.labels.getBoolLabel(false) << '\n';

            gen.code.text << gen.labels.getBoolLabel() << ":\n";
            gen.code.text << "mov " << gen.bit << "ax, 1\n";
            gen.code.text << "jmp " << gen.labels.getLastLabel(false) << '\n';
            gen.labels.addBoolLabel();
        }

        void operator()(const Node::BoolExprOr* expr){
            if(std::holds_alternative<Node::BoolTerm*>(expr->lhs))
                gen.GenBoolTerm(std::get<Node::BoolTerm*>(expr->lhs), gen.bit + "8");
            else
                gen.GenBoolExpr(std::get<Node::BoolExpr*>(expr->lhs), gen.bit + "8");

            if(std::holds_alternative<Node::BoolTerm*>(expr->rhs))
                gen.GenBoolTerm(std::get<Node::BoolTerm*>(expr->rhs), gen.bit + "9");
            else
                gen.GenBoolExpr(std::get<Node::BoolExpr*>(expr->rhs), gen.bit + "9");

            gen.code.text << "cmp " << gen.bit << "8, 1" << '\n';
            gen.code.text << "je " << gen.labels.getBoolLabel(false) << '\n';
            gen.code.text << "cmp " << gen.bit << "9, 1" << '\n';
            gen.code.text << "je " << gen.labels.getBoolLabel(false) << '\n';
            gen.code.text << "jmp " << gen.labels.getCurrentLabel(false) << '\n';

            gen.code.text << gen.labels.getBoolLabel() << ":\n";
            gen.code.text << "mov " << gen.bit << "ax, 1\n";
            gen.code.text << "jmp " << gen.labels.getLastLabel(false) << '\n';
            gen.labels.addBoolLabel();
        }
    };

    BoolExprVisitor visitor(*this);
    std::visit(visitor, expr->expr);

    if(continue_last_label) {
        code.text << labels.getLastLabel() << ":\n";
        labels.addCurrentLabel();
    }

    if(reg != bit + "ax")
        code.text << "mov " << reg << ", " << bit << "ax\n";
}

void Generator::Generate(const Node::Stmt* stmt) {

    struct ProgVisitor {
        Generator& gen;
        ProgVisitor(Generator& generator) : gen(generator) {}

        void operator()(const Node::Exit* stmt){
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
                case PLATFORM_LINUX64:
                    gen.code.text << "mov " << gen.bit << "di, " << gen.bit << "ax\n";
                    gen.code.text << "mov " <<  gen.bit << "ax, 60\n";
                    gen.code.text << "syscall\n";
                    break;
            }
        }

        void operator()(const Node::Link* stmt){
            gen.prg_links.emplace_back(stmt->value->value);
        }

        void operator()(const Node::Variable* stmt){
            gen.storage.StoreVariable(stmt->ident->value, gen.isExprInit(stmt->expr), stmt->type);

            switch(stmt->type){
                case VarType::_char:
                    gen.GenExpr(std::get<Node::IntExpr*>(stmt->expr->expr), "al");
                    gen.code.text << "sub " << gen.bit << "sp, 8\n";
                    gen.code.text << "mov [" << gen.bit << "sp], al\n";
                    break;
                case VarType::_short:
                    gen.GenExpr(std::get<Node::IntExpr*>(stmt->expr->expr), "ax");
                    gen.code.text << "sub " << gen.bit << "sp, 16\n";
                    gen.code.text << "mov [" << gen.bit << "sp], ax\n";
                    break;
                case VarType::_int:
                    gen.GenExpr(std::get<Node::IntExpr*>(stmt->expr->expr), "eax");
                    gen.code.text << "sub " << gen.bit << "sp, 32\n";
                    gen.code.text << "mov [" << gen.bit << "sp], eax\n";
                    break;
                case VarType::_long:
                    if(gen.bit != "r"){
                        Log::Error("You cant have an long/int64 in a 32-bit program");
                        exit(1);
                    }
                    gen.GenExpr(std::get<Node::IntExpr*>(stmt->expr->expr), "rax");
                    gen.code.text << "sub rsp, 64\n";
                    gen.code.text << "mov [rsp], rax\n";
                    break;
                case VarType::_bool:
                    gen.GenBoolExpr(std::get<Node::BoolExpr*>(stmt->expr->expr), gen.bit + "ax");
                    gen.code.text << "sub " << gen.bit << "sp, 8\n";
                    gen.code.text << "mov [" << gen.bit << "sp], " << gen.bit << "ax\n";
                    break;
            }
        }

        void operator()(const Node::Reassign* stmt){
            gen.GenExpr(stmt->expr, gen.bit + "ax");

            uint64_t pos = gen.storage.GetStackPosition(stmt->ident->value);
            gen.code.text << "mov [" << gen.bit << "sp + " << pos << "], " << gen.bit << "ax\n";
        }

        void operator()(const Node::Scope* stmt){
            gen.storage.CreateScope();

            for(Node::Stmt* stmts : stmt->stmts)
                gen.Generate(stmts);

            uint64_t scope_size = gen.storage.EndScope();
            if(scope_size > 0)
                gen.code.text << "add " << gen.bit << "sp, " << scope_size << '\n';
        }

        void operator()(const Node::Assembly* stmt){
            switch(stmt->section){
                case Node::Asm_Section::external:
                    gen.code.external << "extern " << stmt->code->value << '\n';
                    break;
                case Node::Asm_Section::text:
                    gen.code.text << stmt->code->value << '\n';
                    break;
                case Node::Asm_Section::data:
                    gen.code.data << stmt->code->value << '\n';
                    break;
                case Node::Asm_Section::bss:
                    gen.code.bbs << stmt->code->value << '\n';
                    break;
            }
        }

        void operator()(const Node::If* stmt) {
            if (gen.isNextNodeIfChain()) {
                gen.GenBoolExpr(stmt->expr, gen.bit + "ax");
                gen.code.text << "cmp " << gen.bit << "ax, 0\n";
                gen.code.text << "je " << gen.labels.getIfLabel(false) << '\n';    // if false
                gen.code.text << "jmp " << gen.labels.getChainLabel(false) << '\n'; // if true

                gen.code.text << gen.labels.getChainLabel() << ":\n";
                gen.labels.addChainLabel();
                uint64_t if_label_amount = gen.labels.getLabelAmount(Label::LabelType::_if);
                gen.labels.addIfLabel();

                Node::Stmt* send_stmt = new Node::Stmt();
                send_stmt->stmt = stmt->stmt;
                gen.Generate(send_stmt); // generates the scope
                free(send_stmt);

                gen.code.text << "if" << if_label_amount << ":\n";
                gen.labels.setLabelAsCurrent(Label::LabelType::_if);
                gen.labels.addIfLabel();

                gen.index++;
                gen.Generate(gen.prg->prg.at(gen.index));
            }
            else{
                gen.GenBoolExpr(stmt->expr, gen.bit + "ax");
                gen.code.text << "cmp " << gen.bit << "ax, 0\n";
                gen.code.text << "je " << gen.labels.getCurrentLabel(false) << '\n'; // its 0/false
                gen.code.text << "jmp" << gen.labels.getIfLabel(false) << '\n';

                gen.code.text << gen.labels.getIfLabel() << ":\n";
                gen.labels.addIfLabel();

                Node::Stmt* send_stmt = new Node::Stmt();
                send_stmt->stmt = stmt->stmt;
                gen.Generate(send_stmt); // generates the scope
                free(send_stmt);

                gen.code.text << "jmp" << gen.labels.getCurrentLabel(false) << '\n';

                gen.code.text << gen.labels.getCurrentLabel() << ":\n";
                gen.labels.addCurrentLabel();
            }
        }

        void operator()(const Node::Elif* stmt) {
            if (!std::holds_alternative<Node::If*>(gen.prg->prg.at(gen.index - 1)->stmt) &&
                !std::holds_alternative<Node::Elif*>(gen.prg->prg.at(gen.index - 1)->stmt)) {
                Log::Error("An if statement is required before an else if condition statement");
                exit(1);
            }

            if (gen.isNextNodeIfChain()) {
                gen.GenBoolExpr(stmt->expr, gen.bit + "ax");
                gen.code.text << "cmp " << gen.bit << "ax, 0\n";
                gen.code.text << "je " << gen.labels.getIfLabel(false) << '\n';    // if false
                gen.code.text << "jmp" << gen.labels.getChainLabel(false) << '\n'; // if true
                gen.code.text << gen.labels.getChainLabel() << ":\n";
                gen.labels.addChainLabel();
                uint64_t if_label_amount = gen.labels.getLabelAmount(Label::LabelType::_if);
                gen.labels.addIfLabel();

                Node::Stmt* send_stmt = new Node::Stmt();
                send_stmt->stmt = stmt->stmt;
                gen.Generate(send_stmt); // generates the scope
                free(send_stmt);

                gen.code.text << "if" << if_label_amount << ":\n";
                gen.labels.setLabelAsCurrent(Label::LabelType::_if);
                gen.labels.addIfLabel();

                gen.index++;
                gen.Generate(gen.prg->prg.at(gen.index));
            }
            else{
                gen.GenBoolExpr(stmt->expr, gen.bit + "ax");
                gen.code.text << "cmp " << gen.bit << "ax, 0\n";
                gen.code.text << "je " << gen.labels.getMainLabel(false) << '\n'; // its 0/false
                gen.code.text << "jmp " << gen.labels.getChainLabel(false) << '\n';

                gen.code.text << gen.labels.getChainLabel() << ":\n";
                gen.labels.addChainLabel();

                Node::Stmt* send_stmt = new Node::Stmt();
                send_stmt->stmt = stmt->stmt;
                gen.Generate(send_stmt); // generates the scope
                free(send_stmt);

                gen.code.text << "jmp " << gen.labels.getMainLabel(false) << '\n';
                gen.code.text << gen.labels.getMainLabel() << ":\n";
                gen.labels.addMainLabel();
            }
        }

        void operator()(const Node::Else* stmt){
            if(!std::holds_alternative<Node::If*>(gen.prg->prg.at(gen.index - 1)->stmt) &&
               !std::holds_alternative<Node::Elif*>(gen.prg->prg.at(gen.index - 1)->stmt)){
                Log::Error("An if statement is required before an else condition statement");
                exit(1);
            }

            gen.code.text << "jmp " << gen.labels.getChainLabel(false) << '\n';

            gen.code.text << gen.labels.getChainLabel() << ":\n";
            gen.labels.addChainLabel();

            Node::Stmt* send_stmt = new Node::Stmt();
            send_stmt->stmt = stmt->stmt;
            gen.Generate(send_stmt); // generates the scope
            free(send_stmt);

            gen.code.text << "jmp " << gen.labels.getMainLabel(false) << '\n';
            gen.code.text << gen.labels.getMainLabel() << ":\n";
            gen.labels.addMainLabel();
        }

        void operator()(const Node::While* stmt){
            std::string temp_label = gen.labels.getTempLabel(false);
            gen.code.text << "jmp " << temp_label << '\n';
            gen.code.text << gen.labels.getTempLabel() << ":\n";
            gen.labels.addTempLabel();

            gen.GenBoolExpr(stmt->expr, gen.bit + "ax", false);
            gen.code.text << "cmp " << gen.bit << "ax, 1\n";
            gen.code.text << "je " << gen.labels.getLoopLabel(false) << '\n';
            gen.code.text << "jmp " << gen.labels.getLastLabel() << '\n';

            gen.code.text << gen.labels.getLoopLabel() << ":\n";
            gen.labels.addLoopLabel();

            if(stmt->scope.has_value()){
                auto send_stmt = new Node::Stmt();
                send_stmt->stmt = stmt->scope.value();

                gen.Generate(send_stmt);
                free(send_stmt);
            }

            gen.code.text << "jmp " << temp_label << '\n';

            gen.code.text << gen.labels.getLastLabel() << ":\n";
            gen.labels.addLastLabel();
        }
    };

    ProgVisitor visitor(*this);
    std::visit(visitor, stmt->stmt);
}

std::vector<std::string> Generator::GetLinkPrograms() {
    return prg_links;
}

bool Generator::isExprInit(const Node::Expr *expr) {
    return expr != NULL;
}