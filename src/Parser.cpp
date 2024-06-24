#include "Parser.h"

Node::Term* Parser::parseTerm()  {
    if(getNextToken() != TokenType::ident && getNextToken() != TokenType::lit_int &&
    getNextToken() != TokenType::expr_open){
        Log::Error("Expected a term (identifier or integer literal)");
        exit(1);
    }
    Node::Term* term = m_allocator.alloc<Node::Term>();

    if(getNextToken() == TokenType::ident){
        Node::Ident* id = m_allocator.alloc<Node::Ident>();
        id->value = tokens.at(index).value.value();
        term->term = id;
    }
    else if(getNextToken() == TokenType::lit_int){
        Node::LitInt* litInt = m_allocator.alloc<Node::LitInt>();
        litInt->value = tokens.at(index).value.value();
        term->term = litInt;
    }
    else if(getNextToken() == TokenType::expr_open){
        index++;
        auto expr = parseIntExpr(0);
        if(getNextToken() != TokenType::expr_close){
            Log::Error("Expected an `)`");
            exit(1);
        }
        auto paren = m_allocator.alloc<Node::TermParen>();
        paren->expr = expr;
        term->term = paren;
    }
    else{
        Log::Error("Expected an int expression");
        exit(1);
    }

    return term;
}

bool Parser::isBinOp(const TokenType type) {
    switch (type) {
        case TokenType::plus:
        case TokenType::minus:
        case TokenType::star:
        case TokenType::slash:
        case TokenType::percent:
            return true;
        default:
            return false;
    }
}

int Parser::getBinPrec(const TokenType type) {
    switch(type) {
        case TokenType::plus:
        case TokenType::minus:
            return 0;
        case TokenType::star:
        case TokenType::slash:
        case TokenType::percent:
            return 1;
        default:
            return -1;
    }
}

Node::Expr* Parser::parseIntExpr(const int min_prec = 0)  {
    Node::Term* term_lhs = parseTerm();
    auto expr_lhs = m_allocator.alloc<Node::Expr>();
    expr_lhs->var = term_lhs;
    index++;

    while (true) {
        if (index >= tokens.size() || !isBinOp(getNextToken()))
            break;

        checkIfLastToken("Expected a binary operator");

        TokenType op = getNextToken();
        int prec = getBinPrec(op);
        if (prec < min_prec)
            break;

        int next_min_prec = prec + 1;

        index++;
        auto expr_rhs = parseIntExpr(next_min_prec);

        auto expr = m_allocator.alloc<Node::BinExpr>();
        auto lhs = m_allocator.alloc<Node::Expr>();

        lhs->var = expr_lhs->var;

        if (op == TokenType::plus) {
            auto add = m_allocator.alloc<Node::BinExprAdd>();
            add->lhs = lhs;
            add->rhs = expr_rhs;
            expr->expr = add;
        } else if (op == TokenType::minus) {
            auto sub = m_allocator.alloc<Node::BinExprSub>();
            sub->lhs = lhs;
            sub->rhs = expr_rhs;
            expr->expr = sub;
        } else if (op == TokenType::star) {
            auto mul = m_allocator.alloc<Node::BinExprMul>();
            mul->lhs = lhs;
            mul->rhs = expr_rhs;
            expr->expr = mul;
        } else if (op == TokenType::slash) {
            auto div = m_allocator.alloc<Node::BinExprDiv>();
            div->lhs = lhs;
            div->rhs = expr_rhs;
            expr->expr = div;
        } else if (op == TokenType::percent) {
            auto modulo = m_allocator.alloc<Node::BinExprMod>();
            modulo->lhs = lhs;
            modulo->rhs = expr_rhs;
            expr->expr = modulo;
        } else {
            Log::Error("Failed to parse the int expression");
            exit(1);
        }

        // Update expr_lhs to the newly created expression
        expr_lhs = m_allocator.alloc<Node::Expr>();
        expr_lhs->var = expr;
    }

    return expr_lhs;
}

TokenType Parser::getNextToken(const bool newline) {
    if(tokens.at(index).type == TokenType::new_line) {
        if (newline || index + 1 >= tokens.size())
            return TokenType::new_line;
        else {
            index++;
            return getNextToken();
        }
    }
    return tokens.at(index).type;
}

void Parser::checkIfLastToken(const char *msg) {
    if(index >= tokens.size()){
        Log::Error(msg);
    }
}

std::optional<Node::Stmt*> Parser::parseStmt(){
    /// EXIT
    if(getNextToken() == TokenType::exit){
        checkIfLastToken("Expected an expression after exit token");
        index++;

        if(getNextToken() == TokenType::expr_open){
            checkIfLastToken("Expected an exit code after exit(");
            index++;

            Node::Expr* expr = parseIntExpr();
            if(getNextToken() == TokenType::expr_close){
                checkIfLastToken("Expected `;` after exit statement");
                index++;
                if(getNextToken() == TokenType::semi){
                    Node::Exit* e = m_allocator.alloc<Node::Exit>();
                    e->expr = expr;

                    auto stmt = m_allocator.alloc<Node::Stmt>();
                    stmt->stmt = e;
                    return stmt;
                }
                else{
                    Log::Error("Expected a `;` at the end of the exit statement");
                    exit(1);
                }
            }
            else{
                Log::Error("Expected a `)` after the expression of the exit statement");
                exit(1);
            }
        }
        else{
            Log::Error("Expected a `(` to call function exit");
            exit(1);
        }
    }

    /// CREATING INT VARIABLES
    else if(getNextToken() == TokenType::_int16 || getNextToken() == TokenType::_int32 ||
            getNextToken() == TokenType::_int64){

        VarType type;
        switch(getNextToken()){
            case TokenType::_int16:
                type = VarType::_int16;
                break;
            case TokenType::_int32:
                type = VarType::_int32;
                break;
            case TokenType::_int64:
                type = VarType::_int64;
                break;
            default:
                exit(1); // unreachable
        }
        index++;
        checkIfLastToken("Expected an identifier after the `int` keyword for variable declaration");

        if(getNextToken() == TokenType::ident){
            Node::Ident* ident = m_allocator.alloc<Node::Ident>();
            ident->value = tokens.at(index).value.value();
            index++;
            checkIfLastToken("Expected a `;` or initialization of the ident");

            if(getNextToken() == TokenType::semi){
                index++;
                auto var = m_allocator.alloc<Node::Variable>();
                auto term = m_allocator.alloc<Node::Term>();
                auto value = m_allocator.alloc<Node::Ident>();
                value->value = "";
                term->term = value;

                var->type = type;
                var->expr->var = term;
                var->ident = ident;

                auto stmt = m_allocator.alloc<Node::Stmt>();
                stmt->stmt = var;
                return stmt;
            }
            else if(getNextToken() == TokenType::equal){
                index++;
                checkIfLastToken(std::string("Expected an int value to give to identifier `" + ident->value + "`").c_str());

                auto expr = parseIntExpr();
                if(getNextToken() == TokenType::semi){
                    auto var = m_allocator.alloc<Node::Variable>();
                    var->type = type;
                    var->expr = expr;
                    var->ident = ident;

                    auto stmt = m_allocator.alloc<Node::Stmt>();
                    stmt->stmt = var;
                    return stmt;
                }
                else{
                    Log::Error("Expected a `;` after declaring an int variable");
                    exit(1);
                }
            }
            else{
                Log::Error("Expected a `=` or `;` after giving the name of the identifier to finish the statement");
                exit(1);
            }
        }
        else{
            Log::Error("Expected an identifier to declare an int variable");
            exit(1);
        }
    }

    /// HASH COMMANDS
    else if(getNextToken() == TokenType::hash){
        index++;
        checkIfLastToken("Expected an action name after the `#`");

        if(getNextToken() == TokenType::link){
            index++;
            checkIfLastToken("Expected a string for a link file but reached the end of file");

            if(getNextToken() == TokenType::lit_string){
                index++;
                if(getNextToken(true) == TokenType::new_line){
                    auto link = m_allocator.alloc<Node::Link>();
                    auto s = m_allocator.alloc<Node::LitString>();
                    s->value = tokens.at(index - 1).value.value();
                    link->value = s;

                    auto stmt = m_allocator.alloc<Node::Stmt>();
                    stmt->stmt = link;
                    return stmt;
                }
                else{
                    Log::Error("Expected a new line after `#link` statement");
                    exit(1);
                }
            }
            else{
                Log::Error("Expected the link file as a literal string");
                exit(1);
            }
        }
        else if(getNextToken() == TokenType::_extern){
            index++;
            checkIfLastToken("Expected an identifier name to import externally");
            if(getNextToken() != TokenType::ident){
                Log::Error("Expected an identifier name to import externally");
                exit(1);
            }

            auto string = m_allocator.alloc<Node::LitString>();
            string->value = tokens.at(index).value.value();

            index++;
            if(getNextToken(true) != TokenType::new_line){
                Log::Error("Expected a new line after #extern command but got an unexpected token");
                exit(1);
            }

            auto assembly = m_allocator.alloc<Node::Assembly>();
            assembly->code = string;
            assembly->section = Node::Asm_Section::external;

            auto stmt = m_allocator.alloc<Node::Stmt>();
            stmt->stmt = assembly;

            return stmt;
        }
        else{
            Log::Info("hash command not implemented");
        }
    }

    /// SCOPES
    else if(getNextToken() == TokenType::scope_open){
        checkIfLastToken("Un-closed scope, expected `}`");

        auto scope = m_allocator.alloc<Node::Scope>();
        index++;
        while(auto stmt = parseStmt()){
            scope->stmts.emplace_back(stmt.value());
            index++;
        }

        if(getNextToken() == TokenType::scope_close){
            auto stmt = m_allocator.alloc<Node::Stmt>();
            stmt->stmt = scope;
            return stmt;
        }

        Log::Error("Un-closed scope, expected `}`");
        exit(1);
    }

    /// IF
    else if(getNextToken() == TokenType::_if){
        checkIfLastToken("Expected `(` after if keyword with an expr");
        index++;

        if(getNextToken() != TokenType::expr_open){
            Log::Error("Expected an `(` after if keyword");
            exit(1);
        }
        index++;

        auto _if = m_allocator.alloc<Node::If>();
        auto expr = parseIntExpr();

        // TODO: check for `==`, `>`, `<` and more!

        if(getNextToken() != TokenType::expr_close){
            Log::Error("Expected `)` at the end of the if condition");
            exit(1);
        }
        index++;

        auto stmt = parseStmt();

        if(!std::holds_alternative<Node::Scope*>(stmt.value()->stmt)) {
            auto scope = m_allocator.alloc<Node::Scope>();
            scope->stmts.emplace_back(stmt.value());
            _if->stmt = scope;
        }
        else{
            _if->stmt = std::get<Node::Scope*>(stmt.value()->stmt);
        }

        _if->expr = expr;

        auto ret_stmt = m_allocator.alloc<Node::Stmt>();
        ret_stmt->stmt = _if;

        return ret_stmt;
    }

    /// ELSE AND ELSE IF
    else if(getNextToken() == TokenType::_else){
        index++;
        checkIfLastToken("Expected a statement after keyword else");

        /// Else if
        if(getNextToken() == TokenType::_if){
            index++;
            checkIfLastToken("Expected a condition after the else if statement");

            if(getNextToken() != TokenType::expr_open){
                Log::Error("Expected an `(` after else if");
                exit(1);
            }
            index++;
            checkIfLastToken("Expected a condition to check after `else if(`");

            auto expr = parseIntExpr();

            // TODO: check for `==`, `>`, `<` and more!

            if(getNextToken() != TokenType::expr_close){
                Log::Error("Expected `)` at the end of the else if condition");
                exit(1);
            }
            index++;
            checkIfLastToken("Expected a statement to execute after the else if condition");


            auto stmt = parseStmt(); // guaranteed a value
            auto scope = m_allocator.alloc<Node::Scope>();

            if(!std::holds_alternative<Node::Scope*>(stmt.value()->stmt))
                scope->stmts.emplace_back(stmt.value());
            else
                scope = std::get<Node::Scope*>(stmt.value()->stmt);

            auto else_stmt = m_allocator.alloc<Node::Elif>();
            else_stmt->stmt = scope;
            else_stmt->expr = expr;

            auto return_stmt = new Node::Stmt();
            return_stmt->stmt = else_stmt;
            return return_stmt;
        }

        /// Else
        auto stmt = parseStmt(); // guaranteed a value
        auto scope = m_allocator.alloc<Node::Scope>();

        if(!std::holds_alternative<Node::Scope*>(stmt.value()->stmt))
            scope->stmts.emplace_back(stmt.value());
        else
            scope = std::get<Node::Scope*>(stmt.value()->stmt);

        auto else_stmt = m_allocator.alloc<Node::Else>();
        else_stmt->stmt = scope;

        auto return_stmt = new Node::Stmt();
        return_stmt->stmt = else_stmt;
        return return_stmt;
    }

    /// REASSIGNMENT
    else if (getNextToken() == TokenType::ident) {
        std::string identValue = tokens.at(index).value.value();
        index++;
        checkIfLastToken(("Expected something after identifier" + identValue).c_str());

        Node::Reassign* stmt = m_allocator.alloc<Node::Reassign>();
        Node::Ident* ident = m_allocator.alloc<Node::Ident>();
        ident->value = identValue;
        stmt->ident = ident;

        TokenType nextToken = getNextToken();
        switch (nextToken) {
            case TokenType::plus:
                index++;
                if (getNextToken() == TokenType::plus) {
                    // Parse ident++
                    index++;
                    Node::BinExpr* binExpr = m_allocator.alloc<Node::BinExpr>();
                    Node::BinExprAdd* binExprAdd = m_allocator.alloc<Node::BinExprAdd>();
                    Node::Expr* lhs = m_allocator.alloc<Node::Expr>();
                    Node::Expr* rhs = m_allocator.alloc<Node::Expr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    Node::Term* rhsTerm = m_allocator.alloc<Node::Term>();
                    Node::LitInt* rhsInt = m_allocator.alloc<Node::LitInt>();
                    rhsInt->value = "1";

                    lhsTerm->term = ident;
                    rhsTerm->term = rhsInt;
                    lhs->var = lhsTerm;
                    rhs->var = rhsTerm;
                    binExprAdd->lhs = lhs;
                    binExprAdd->rhs = rhs;
                    binExpr->expr = binExprAdd;

                    Node::Expr* expr = m_allocator.alloc<Node::Expr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else if (getNextToken() == TokenType::equal) {
                    // Parse ident += expr
                    index++;
                    Node::BinExpr* binExpr = m_allocator.alloc<Node::BinExpr>();
                    Node::BinExprAdd* binExprAdd = m_allocator.alloc<Node::BinExprAdd>();
                    Node::Expr* lhs = m_allocator.alloc<Node::Expr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::Expr* rhs = parseIntExpr();
                    binExprAdd->lhs = lhs;
                    binExprAdd->rhs = rhs;
                    binExpr->expr = binExprAdd;

                    Node::Expr* expr = m_allocator.alloc<Node::Expr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `+`");
                    exit(1);
                }
                break;

            case TokenType::minus:
                index++;
                if (getNextToken() == TokenType::minus) {
                    // Parse ident--
                    index++;
                    Node::BinExpr* binExpr = m_allocator.alloc<Node::BinExpr>();
                    Node::BinExprSub* binExprSub = m_allocator.alloc<Node::BinExprSub>();
                    Node::Expr* lhs = m_allocator.alloc<Node::Expr>();
                    Node::Expr* rhs = m_allocator.alloc<Node::Expr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    Node::Term* rhsTerm = m_allocator.alloc<Node::Term>();
                    Node::LitInt* rhsInt = m_allocator.alloc<Node::LitInt>();
                    rhsInt->value = "1";

                    lhsTerm->term = ident;
                    rhsTerm->term = rhsInt;
                    lhs->var = lhsTerm;
                    rhs->var = rhsTerm;
                    binExprSub->lhs = lhs;
                    binExprSub->rhs = rhs;
                    binExpr->expr = binExprSub;

                    Node::Expr* expr = m_allocator.alloc<Node::Expr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else if (getNextToken() == TokenType::equal) {
                    // Parse ident -= expr
                    index++;
                    Node::BinExpr* binExpr = m_allocator.alloc<Node::BinExpr>();
                    Node::BinExprSub* binExprSub = m_allocator.alloc<Node::BinExprSub>();
                    Node::Expr* lhs = m_allocator.alloc<Node::Expr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::Expr* rhs = parseIntExpr();
                    binExprSub->lhs = lhs;
                    binExprSub->rhs = rhs;
                    binExpr->expr = binExprSub;

                    Node::Expr* expr = m_allocator.alloc<Node::Expr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `-`");
                    exit(1);
                }
                break;

            case TokenType::star:
                index++;
                if (getNextToken() == TokenType::equal) {
                    // Parse ident *= expr
                    index++;
                    Node::BinExpr* binExpr = m_allocator.alloc<Node::BinExpr>();
                    Node::BinExprMul* binExprMul = m_allocator.alloc<Node::BinExprMul>();
                    Node::Expr* lhs = m_allocator.alloc<Node::Expr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::Expr* rhs = parseIntExpr();
                    binExprMul->lhs = lhs;
                    binExprMul->rhs = rhs;
                    binExpr->expr = binExprMul;

                    Node::Expr* expr = m_allocator.alloc<Node::Expr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `*`");
                    exit(1);
                }
                break;

            case TokenType::slash:
                index++;
                if (getNextToken() == TokenType::equal) {
                    // Parse ident /= expr
                    index++;
                    Node::BinExpr* binExpr = m_allocator.alloc<Node::BinExpr>();
                    Node::BinExprDiv* binExprDiv = m_allocator.alloc<Node::BinExprDiv>();
                    Node::Expr* lhs = m_allocator.alloc<Node::Expr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::Expr* rhs = parseIntExpr();
                    binExprDiv->lhs = lhs;
                    binExprDiv->rhs = rhs;
                    binExpr->expr = binExprDiv;

                    Node::Expr* expr = m_allocator.alloc<Node::Expr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `/`");
                    exit(1);
                }
                break;

            case TokenType::percent:
                index++;
                if (getNextToken() == TokenType::equal) {
                    // Parse ident %= expr
                    index++;
                    Node::BinExpr* binExpr = m_allocator.alloc<Node::BinExpr>();
                    Node::BinExprMod* binExprMod = m_allocator.alloc<Node::BinExprMod>();
                    Node::Expr* lhs = m_allocator.alloc<Node::Expr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::Expr* rhs = parseIntExpr();
                    binExprMod->lhs = lhs;
                    binExprMod->rhs = rhs;
                    binExpr->expr = binExprMod;

                    Node::Expr* expr = m_allocator.alloc<Node::Expr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `%`");
                    exit(1);
                }
                break;

            case TokenType::equal:
                // Parse ident = expr
                index++;
                stmt->expr = parseIntExpr();
                break;

            default:
                Log::Error("Unexpected token after ident");
                exit(1);
        }

        if (getNextToken() != TokenType::semi) {
            Log::Error("Expected a `;`");
            exit(1);
        }

        Node::Stmt* returnStmt = m_allocator.alloc<Node::Stmt>();
        returnStmt->stmt = stmt;
        return returnStmt;
    }

    /// ASSEMBLY INSIDE THE PROGRAM
    /// has instructions for modifying assembly directly from the program
    /// ASM .text, .data, .bss and external functions
    else if(getNextToken() == TokenType::_asm_text || getNextToken() == TokenType::_asm_data || getNextToken() == TokenType::_asm_bss){
        Node::Asm_Section section;

        switch(getNextToken()){
            case TokenType::_asm_text:
                section = Node::Asm_Section::text;
                break;
            case TokenType::_asm_data:
                section = Node::Asm_Section::data;
                break;
            case TokenType::_asm_bss:
                section = Node::Asm_Section::bss;
                break;
            default:
                exit(1); // unreachable, used for disabling warning messages
        }

        index++;
        checkIfLastToken("Expected a `<` to stream into the output assembly but reached the last token");
        if(getNextToken() != TokenType::less_then){
            Log::Error("Expected a `<` to stream into the output assembly");
            exit(1);
        }

        index++;
        checkIfLastToken("Expected a string value to stream into the output assembly but reached the last token");
        if(getNextToken() != TokenType::lit_string){
            Log::Error("Expected a string value to stream into the output assembly");
            exit(1);
        }

        auto code = m_allocator.alloc<Node::LitString>();
        code->value = tokens.at(index).value.value();

        index++;
        if(getNextToken() != TokenType::semi){
            Log::Error("Expected a `;` after the assembly statement");
            exit(1);
        }

        auto assembly = m_allocator.alloc<Node::Assembly>();
        assembly->section = section;
        assembly->code = code;

        auto stmt = m_allocator.alloc<Node::Stmt>();
        stmt->stmt = assembly;

        return stmt;
    }

    /// END OF TOKENS OR END OF SCOPE
    if(getNextToken() == TokenType::new_line || getNextToken() == TokenType::scope_close)
        return {};
    Log::Error("Unexpected token");
    exit(1);
}

/// public

Node::Program *Parser::parse() {

    for(index = 0; index < tokens.size(); index++){
        auto stmt = parseStmt();
        if(stmt.has_value()) {
            program.prg.emplace_back(stmt.value());
        }
        else if(getNextToken() == TokenType::scope_close){ // needed for parsing the scopes
            Log::Error("Unexpected `}`");
            exit(1);
        }
        else
            break;
    }

    return &program;
}
