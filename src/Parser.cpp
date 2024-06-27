#include "Parser.h"

Node::Term* Parser::parseTerm()  {
    checkIfLastToken("Expected an integer term");
    if(getNextToken() != TokenType::ident && getNextToken() != TokenType::lit_int &&
    getNextToken() != TokenType::expr_open){
        Log::Error("Expected a term (identifier or integer literal) at " + getNextTokenPos());
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
            Log::Error("Expected an `)` at" + getNextTokenPos());
            exit(1);
        }
        auto paren = m_allocator.alloc<Node::TermParen>();
        paren->expr = expr;
        term->term = paren;
    }
    else{
        Log::Error("Expected an int expression at " + getNextTokenPos());
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

Node::BoolTerm* Parser::parseBoolTerm(){
    auto term = m_allocator.alloc<Node::BoolTerm>();
    auto term_int = m_allocator.alloc<Node::BoolTermInt>();
    bool has_rhs_expr = true;
    Node::IntExpr* lhs = parseIntExpr();
    term_int->lhs = lhs;
    checkIfLastToken("Expected a comparison operator after the int expression");
    switch(getNextToken()) {
        case TokenType::equal:{
            index++;
            checkIfLastToken("Expected something after comparison operator equals");
            if (getNextToken() != TokenType::equal) {
                Log::Error("Expected something after comparison operator equals at " + getNextTokenPos());
                exit(1);
            }
            index++;
            checkIfLastToken("Expected something after `==`");
            term_int->comp = Node::Comparison::equal;
            break;
        }

        case TokenType::_not: {
            index++;
            checkIfLastToken("Expected something after comparison operator `!`");
            if (getNextToken() != TokenType::equal) {
                Log::Error("Expected something after comparison operator `!` at " + getNextTokenPos());
                exit(1);
            }
            index++;
            checkIfLastToken("Expected something after `!=`");
            term_int->comp = Node::Comparison::not_equal;
            break;
        }

        case TokenType::greater_then: {
            index++;
            checkIfLastToken("Expected something after comparison operator `>`");
            if (getNextToken() != TokenType::equal) {
                term_int->comp = Node::Comparison::greater;
                index++;
                checkIfLastToken("Expected something after `>`");
            } else {
                term_int->comp = Node::Comparison::greater_equal;
                index++;
                checkIfLastToken("Expected something after `>=`");
            }

            break;
        }

        case TokenType::less_then:{
            index++;
            checkIfLastToken("Expected something after comparison operator `<`");
            if (getNextToken() != TokenType::equal) {
                term_int->comp = Node::Comparison::less;
                index++;
                checkIfLastToken("Expected something after `<`");
            } else {
                term_int->comp = Node::Comparison::less_equal;
                index++;
                checkIfLastToken("Expected something after `<=`");
            }

            break;
        }

        case TokenType::expr_open: {
            index++;
            checkIfLastToken("Expected something after `(`");
            auto bool_expr = parseBoolExpr();

            if(getNextToken() != TokenType::expr_close){
                Log::Error("Expected `)`");
                exit(1);
            }

            auto term_paren = m_allocator.alloc<Node::BoolTermParen>();
            term_paren->expr = bool_expr;
            term->term = term_paren;

            break;
        }

        default:
            has_rhs_expr = false;
    }

    if(has_rhs_expr) {
        Node::IntExpr *rhs = parseIntExpr();
        term_int->rhs = rhs;
        term->term = term_int;
    }
    else{
        auto rhs = m_allocator.alloc<Node::IntExpr>();
        auto rhs_term = m_allocator.alloc<Node::Term>();
        auto lit_int = m_allocator.alloc<Node::LitInt>();
        lit_int->value = '0';
        rhs_term->term = lit_int;
        rhs->var = rhs_term;
    }

    return term;
}

bool Parser::isLogicOp(){
    if(getNextToken() == TokenType::_and){
        index++;
        checkIfLastToken("Expected another `&` to make an `and` logic operator but reached the last token");
        if(getNextToken() == TokenType::_and)
            return true;
        index--;
    }
    else if(getNextToken() == TokenType::_or){
        index++;
        checkIfLastToken("Expected another `|` to make an `or` logic operator but reached the last token");
        if(getNextToken() == TokenType::_or)
            return true;
        index--;
    }

    return false;
}

Node::BoolExpr* Parser::parseBoolExpr(){
    Node::BoolTerm* term_lhs = parseBoolTerm();
    auto expr_lhs = m_allocator.alloc<Node::BoolExpr>();
    expr_lhs->expr = term_lhs;

    if(index >= tokens.size() || !isLogicOp())
        return expr_lhs;

    bool was_token_and_op = getNextToken() == TokenType::_and;
    index++;
    checkIfLastToken("Expected something after logical operator and, `&&`");

    auto expr_rhs = parseBoolExpr();

    if(was_token_and_op){
        auto return_expr = m_allocator.alloc<Node::BoolExpr>();
        auto and_expr = m_allocator.alloc<Node::BoolExprAnd>();
        and_expr->lhs = expr_lhs;
        and_expr->rhs = expr_rhs;
        return_expr->expr = and_expr;
        return return_expr;
    }
    else{
        auto return_expr = m_allocator.alloc<Node::BoolExpr>();
        auto or_expr = m_allocator.alloc<Node::BoolExprOr>();
        or_expr->lhs = expr_lhs;
        or_expr->rhs = expr_rhs;
        return_expr->expr = or_expr;
        return return_expr;
    }

    return expr_lhs;
}

Node::IntExpr* Parser::parseIntExpr(const int min_prec)  {
    Node::Term* term_lhs = parseTerm();
    auto expr_lhs = m_allocator.alloc<Node::IntExpr>();
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
        auto lhs = m_allocator.alloc<Node::IntExpr>();

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
            Log::Error("Failed to parse the int expression at " + getNextTokenPos());
            exit(1);
        }

        // Update expr_lhs to the newly created expression
        expr_lhs = m_allocator.alloc<Node::IntExpr>();
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

std::string Parser::getNextTokenPos() {
    std::stringstream str;
    str << tokens.at(index).line << ':' << tokens.at(index).col;
    return str.str();
}

void Parser::checkIfLastToken(const char *msg) {
    if(index >= tokens.size()){
        std::stringstream str;
        str << msg << " at " << tokens.at(index - 1).line << ':' << tokens.at(index - 1).col;
        Log::Error(str.str());
        exit(1);
    }
}

std::optional<Node::Stmt*> Parser::parseStmt(){
    /// EXIT
    if(getNextToken() == TokenType::exit){
        index++;
        checkIfLastToken("Expected an expression after exit token");

        if(getNextToken() == TokenType::expr_open){
            index++;
            checkIfLastToken("Expected an exit code after exit(");

            Node::IntExpr* expr = parseIntExpr();
            if(getNextToken() == TokenType::expr_close){
                index++;
                checkIfLastToken("Expected `;` after exit statement");
                if(getNextToken() == TokenType::semi){
                    Node::Exit* e = m_allocator.alloc<Node::Exit>();
                    e->expr = expr;

                    auto stmt = m_allocator.alloc<Node::Stmt>();
                    stmt->stmt = e;
                    return stmt;
                }
                else{
                    Log::Error("Expected a `;` at the end of the exit statement at " + getNextTokenPos());
                    exit(1);
                }
            }
            else{
                Log::Error("Expected a `)` after the expression of the exit statement at " + getNextTokenPos());
                exit(1);
            }
        }
        else{
            Log::Error("Expected a `(` to call function exit at " + getNextTokenPos());
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
                    Log::Error("Expected a `;` after declaring an int variable at " + getNextTokenPos());
                    exit(1);
                }
            }
            else{
                Log::Error("Expected a `=` or `;` after giving the name of the identifier to finish the statement at " + getNextTokenPos());
                exit(1);
            }
        }
        else{
            Log::Error("Expected an identifier to declare an int variable at " + getNextTokenPos());
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
                    Log::Error("Expected a new line after `#link` statement at " + getNextTokenPos());
                    exit(1);
                }
            }
            else{
                Log::Error("Expected the link file as a literal string at " + getNextTokenPos());
                exit(1);
            }
        }
        else if(getNextToken() == TokenType::_extern){
            index++;
            checkIfLastToken("Expected an identifier name to import externally");
            if(getNextToken() != TokenType::ident){
                Log::Error("Expected an identifier name to import externally at " + getNextTokenPos());
                exit(1);
            }

            auto string = m_allocator.alloc<Node::LitString>();
            string->value = tokens.at(index).value.value();

            index++;
            if(getNextToken(true) != TokenType::new_line){
                Log::Error("Expected a new line after #extern command but got an unexpected token at " + getNextTokenPos());
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

        Log::Error("Un-closed scope, expected `}` at " + getNextTokenPos());
        exit(1);
    }

    /// IF
    else if(getNextToken() == TokenType::_if){
        checkIfLastToken("Expected `(` after if keyword with an expr");
        index++;

        if(getNextToken() != TokenType::expr_open){
            Log::Error("Expected an `(` after if keyword at " + getNextTokenPos());
            exit(1);
        }
        index++;

        auto _if = m_allocator.alloc<Node::If>();
        auto expr = parseBoolExpr();

        if(getNextToken() != TokenType::expr_close){
            Log::Error("Expected `)` at the end of the if condition at " + getNextTokenPos());
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
                Log::Error("Expected an `(` after else if at " + getNextTokenPos());
                exit(1);
            }
            index++;
            checkIfLastToken("Expected a condition to check after `else if(`");

            auto expr = parseBoolExpr();

            if(getNextToken() != TokenType::expr_close){
                Log::Error("Expected `)` at the end of the else if condition at " + getNextTokenPos());
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
                    Node::IntExpr* lhs = m_allocator.alloc<Node::IntExpr>();
                    Node::IntExpr* rhs = m_allocator.alloc<Node::IntExpr>();
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

                    Node::IntExpr* expr = m_allocator.alloc<Node::IntExpr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else if (getNextToken() == TokenType::equal) {
                    // Parse ident += expr
                    index++;
                    Node::BinExpr* binExpr = m_allocator.alloc<Node::BinExpr>();
                    Node::BinExprAdd* binExprAdd = m_allocator.alloc<Node::BinExprAdd>();
                    Node::IntExpr* lhs = m_allocator.alloc<Node::IntExpr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::IntExpr* rhs = parseIntExpr();
                    binExprAdd->lhs = lhs;
                    binExprAdd->rhs = rhs;
                    binExpr->expr = binExprAdd;

                    Node::IntExpr* expr = m_allocator.alloc<Node::IntExpr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `+` at " + getNextTokenPos());
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
                    Node::IntExpr* lhs = m_allocator.alloc<Node::IntExpr>();
                    Node::IntExpr* rhs = m_allocator.alloc<Node::IntExpr>();
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

                    Node::IntExpr* expr = m_allocator.alloc<Node::IntExpr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else if (getNextToken() == TokenType::equal) {
                    // Parse ident -= expr
                    index++;
                    Node::BinExpr* binExpr = m_allocator.alloc<Node::BinExpr>();
                    Node::BinExprSub* binExprSub = m_allocator.alloc<Node::BinExprSub>();
                    Node::IntExpr* lhs = m_allocator.alloc<Node::IntExpr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::IntExpr* rhs = parseIntExpr();
                    binExprSub->lhs = lhs;
                    binExprSub->rhs = rhs;
                    binExpr->expr = binExprSub;

                    Node::IntExpr* expr = m_allocator.alloc<Node::IntExpr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `-` at " + getNextTokenPos());
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
                    Node::IntExpr* lhs = m_allocator.alloc<Node::IntExpr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::IntExpr* rhs = parseIntExpr();
                    binExprMul->lhs = lhs;
                    binExprMul->rhs = rhs;
                    binExpr->expr = binExprMul;

                    Node::IntExpr* expr = m_allocator.alloc<Node::IntExpr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `*` at " + getNextTokenPos());
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
                    Node::IntExpr* lhs = m_allocator.alloc<Node::IntExpr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::IntExpr* rhs = parseIntExpr();
                    binExprDiv->lhs = lhs;
                    binExprDiv->rhs = rhs;
                    binExpr->expr = binExprDiv;

                    Node::IntExpr* expr = m_allocator.alloc<Node::IntExpr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `/` at " + getNextTokenPos());
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
                    Node::IntExpr* lhs = m_allocator.alloc<Node::IntExpr>();
                    Node::Term* lhsTerm = m_allocator.alloc<Node::Term>();
                    lhsTerm->term = ident;
                    lhs->var = lhsTerm;
                    Node::IntExpr* rhs = parseIntExpr();
                    binExprMod->lhs = lhs;
                    binExprMod->rhs = rhs;
                    binExpr->expr = binExprMod;

                    Node::IntExpr* expr = m_allocator.alloc<Node::IntExpr>();
                    expr->var = binExpr;
                    stmt->expr = expr;
                } else {
                    Log::Error("Unexpected token after `%` at " + getNextTokenPos());
                    exit(1);
                }
                break;

            case TokenType::equal:
                // Parse ident = expr
                index++;
                stmt->expr = parseIntExpr();
                break;

            default:
                Log::Error("Unexpected token after ident at " + getNextTokenPos());
                exit(1);
        }

        if (getNextToken() != TokenType::semi) {
            Log::Error("Expected a `;` at " + getNextTokenPos());
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
            Log::Error("Expected a `<` to stream into the output assembly at " + getNextTokenPos());
            exit(1);
        }

        index++;
        checkIfLastToken("Expected a string value to stream into the output assembly but reached the last token");
        if(getNextToken() != TokenType::lit_string){
            Log::Error("Expected a string value to stream into the output assembly at " + getNextTokenPos());
            exit(1);
        }

        auto code = m_allocator.alloc<Node::LitString>();
        code->value = tokens.at(index).value.value();

        index++;
        if(getNextToken() != TokenType::semi){
            Log::Error("Expected a `;` after the assembly statement at " + getNextTokenPos());
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
    Log::Error("Unexpected token at " + getNextTokenPos());
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
            Log::Error("Unexpected `}` at " + getNextTokenPos());
            exit(1);
        }
        else
            break;
    }

    return &program;
}
