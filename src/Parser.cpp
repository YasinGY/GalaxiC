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
        std::string ident = tokens.at(index).value.value();
        if(!isIntIdent(ident)){
            Log::Error("Expected an int identifier at " + getNextTokenPos() + " but got a type " + VarTypeToString(
                    getIdentType(ident)));
            exit(1);
        }

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

VarType Parser::getIdentType(const std::string &ident) {
    for(Node::Stmt* stmt : program.prg){
        if(!std::holds_alternative<Node::Variable*>(stmt->stmt) && std::get<Node::Variable*>(stmt->stmt)->ident->value != ident)
            continue;

        return std::get<Node::Variable*>(stmt->stmt)->type;
    }

    Log::Error("Unknown identifier `" + ident + "` at " + getNextTokenPos());
    exit(1);
}

bool Parser::isLitBool(TokenType type) {
    return type == TokenType::_false || type == TokenType::_true;
}

bool Parser::isIntIdent(const std::string &ident) {
    return
    getIdentType(tokens.at(index).value.value()) == VarType::_short ||
    getIdentType(tokens.at(index).value.value()) == VarType::_int ||
    getIdentType(tokens.at(index).value.value()) == VarType::_long;
}

Node::Comparison Parser::parseComparison() {
    switch(getNextToken()) {
        case TokenType::equal: {
            index++;
            checkIfLastToken("Expected something after comparison operator equals");
            if (getNextToken() != TokenType::equal) {
                Log::Error("Expected something after comparison operator equals at " + getNextTokenPos());
                exit(1);
            }
            index++;
            checkIfLastToken("Expected something after `==`");
            return Node::Comparison::equal;
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
            return Node::Comparison::not_equal;
        }

        case TokenType::greater_then: {
            index++;
            checkIfLastToken("Expected something after comparison operator `>`");
            if (getNextToken() != TokenType::equal) {
                checkIfLastToken("Expected something after `>`");
                return Node::Comparison::greater;
            } else {
                index++;
                checkIfLastToken("Expected something after `>=`");
                return Node::Comparison::greater_equal;
            }
        }

        case TokenType::less_then: {
            index++;
            checkIfLastToken("Expected something after comparison operator `<`");
            if (getNextToken() != TokenType::equal) {
                checkIfLastToken("Expected something after `<`");
                return Node::Comparison::less;
            } else {
                index++;
                checkIfLastToken("Expected something after `<=`");
                return Node::Comparison::less_equal;
            }
        }

        default:
            return Node::Comparison::None;
    }
}

Node::BoolTerm* Parser::parseBoolTerm() {
    checkIfLastToken("Expected a boolean expression term");

    auto term = m_allocator.alloc<Node::BoolTerm>();
    Token curr_token = tokens.at(index);

    if(isLitBool(curr_token.type) || (curr_token.type == TokenType::ident && getIdentType(curr_token.value.value()) == VarType::_bool)){
        auto bool_term = m_allocator.alloc<Node::BoolTermBool>();

        if(curr_token.type == TokenType::ident){
            auto ident = m_allocator.alloc<Node::Ident>();
            ident->value = curr_token.value.value();
            bool_term->lhs = ident;
        }
        else if(curr_token.type == TokenType::_true){
            bool_term->lhs = Node::LitBool::_true;
        }
        else{
            bool_term->lhs = Node::LitBool::_false;
        }
        index++;
        checkIfLastToken("Expected a `;` to end the statement");

        Node::Comparison comp = parseComparison();

        if(comp != Node::Comparison::None){
            bool_term->comp = comp;

            if(getNextToken() == TokenType::ident){
                auto rhs = m_allocator.alloc<Node::Ident>();
                rhs->value = tokens.at(index).value.value();
                bool_term->rhs = rhs;
            }
            else if(getNextToken() == TokenType::_true)
                bool_term->rhs = Node::LitBool::_true;
            else if(getNextToken() == TokenType::_false)
                bool_term->rhs = Node::LitBool::_false;
            else{
                Log::Error("Expected a boolean to complete the boolean expression at " + getNextTokenPos());
                exit(1);
            }

            index++;
        }
        else{
            bool_term->comp = Node::Comparison::equal;
            bool_term->rhs = Node::LitBool::_true;
        }

        term->term = bool_term;
    }
    else if(curr_token.type == TokenType::lit_int || (curr_token.type == TokenType::ident && isIntIdent(curr_token.value.value()))){
        auto int_expr = m_allocator.alloc<Node::BoolTermInt>();

        int_expr->lhs = parseIntExpr();
        int_expr->comp = parseComparison();
        int_expr->rhs = parseIntExpr();

        term->term = int_expr;
    }
    else{
        Log::Error("Expected a boolean term but got an unexpected token at " + getNextTokenPos());
        exit(1);
    }

    return term;
}

Node::BoolExpr* Parser::parseBoolExpr(){
    auto expr = m_allocator.alloc<Node::BoolExpr>();
    auto term_lhs = parseBoolTerm();

    if(getNextToken() != TokenType::_and && getNextToken() != TokenType::_or){
        expr->expr = term_lhs;
        return expr;
    }

    if(getNextToken() == TokenType::_and && index + 1 < tokens.size() && tokens.at(index + 1).type == TokenType::_and){
        index++;
        checkIfLastToken("Expected `&&` but only received `&`");
        index++;
        checkIfLastToken("Expected a boolean expression after and operator");

        auto expr_and = m_allocator.alloc<Node::BoolExprAnd>();
        auto rhs = parseBoolExpr();

        expr_and->lhs = term_lhs;
        expr_and->rhs = rhs;
        expr->expr = expr_and;
    }
    else if(getNextToken() == TokenType::_or && index + 1 < tokens.size() && tokens.at(index + 1).type == TokenType::_or){
        index++;
        checkIfLastToken("Expected `||` but only received `|`");
        index++;
        checkIfLastToken("Expected a boolean expression after or operator");

        auto expr_or = m_allocator.alloc<Node::BoolExprOr>();
        auto rhs = parseBoolExpr();

        expr_or->lhs = term_lhs;
        expr_or->rhs = rhs;
        expr->expr = expr_or;
    }
    else{
        // means that the first token as `&` or `|` but the second one was not
        if(getNextToken() == TokenType::_and) {
            index++;
            Log::Error("Expected another `&` at " + getNextTokenPos());
            exit(1);
        }
        else{
            index++;
            Log::Error("Expected another `|` at " + getNextTokenPos());
            exit(1);
        }
    }

    return expr;
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

void Parser::checkIfLastToken(const std::string& msg, bool add_pos) {
    if(index >= tokens.size()){
        std::stringstream str;
        str << msg << " at " << tokens.at(index - 1).line << ':' << tokens.at(index - 1).col;
        Log::Error(str.str());
        exit(1);
    }
}

std::optional<Node::Stmt*> Parser::parseStmt(){
    switch(getNextToken()) {
        /// EXIT
        case TokenType::exit: {
            index++;
            checkIfLastToken("Expected an expression after exit token");

            if (getNextToken() == TokenType::expr_open) {
                index++;
                checkIfLastToken("Expected an exit code after exit(");

                Node::IntExpr *expr = parseIntExpr();
                if (getNextToken() == TokenType::expr_close) {
                    index++;
                    checkIfLastToken("Expected `;` after exit statement");
                    if (getNextToken() == TokenType::semi) {
                        Node::Exit *e = m_allocator.alloc<Node::Exit>();
                        e->expr = expr;

                        auto stmt = m_allocator.alloc<Node::Stmt>();
                        stmt->stmt = e;
                        return stmt;
                    } else {
                        Log::Error("Expected a `;` at the end of the exit statement at " + getNextTokenPos());
                        exit(1);
                    }
                } else {
                    Log::Error("Expected a `)` after the expression of the exit statement at " + getNextTokenPos());
                    exit(1);
                }
            } else {
                Log::Error("Expected a `(` to call function exit at " + getNextTokenPos());
                exit(1);
            }
        }

            /// CREATING INT VARIABLES
        case TokenType::_int16:
        case TokenType::_int32:
        case TokenType::_int64: {

            VarType type;
            switch (getNextToken()) {
                case TokenType::_int16:
                    type = VarType::_short;
                    break;
                case TokenType::_int32:
                    type = VarType::_int;
                    break;
                case TokenType::_int64:
                    type = VarType::_long;
                    break;
                default:
                    exit(1); // unreachable
            }
            index++;
            checkIfLastToken("Expected an identifier after the `int` keyword for variable declaration");

            if (getNextToken() == TokenType::ident) {
                Node::Ident *ident = m_allocator.alloc<Node::Ident>();
                ident->value = tokens.at(index).value.value();
                index++;
                checkIfLastToken("Expected a `;` or initialization of the ident");

                if (getNextToken() == TokenType::semi) {
                    index++;
                    auto var = m_allocator.alloc<Node::Variable>();
                    var->type = type;
                    var->ident = ident;

                    auto stmt = m_allocator.alloc<Node::Stmt>();
                    stmt->stmt = var;
                    return stmt;
                } else if (getNextToken() == TokenType::equal) {
                    index++;
                    checkIfLastToken(
                            std::string("Expected an int value to give to identifier `" + ident->value + "`").c_str());

                    auto int_expr = parseIntExpr();
                    if (getNextToken() == TokenType::semi) {
                        auto var = m_allocator.alloc<Node::Variable>();
                        auto expr = m_allocator.alloc<Node::Expr>();
                        var->type = type;
                        expr->expr = int_expr;
                        var->expr = expr;
                        var->ident = ident;

                        auto stmt = m_allocator.alloc<Node::Stmt>();
                        stmt->stmt = var;
                        return stmt;
                    } else {
                        Log::Error("Expected a `;` after declaring an int variable at " + getNextTokenPos());
                        exit(1);
                    }
                } else {
                    Log::Error(
                            "Expected a `=` or `;` after giving the name of the identifier to finish the statement at " +
                            getNextTokenPos());
                    exit(1);
                }
            } else {
                Log::Error("Expected an identifier to declare an int variable at " + getNextTokenPos());
                exit(1);
            }
        }

        /// BOOLEAN VARIABLES
        case TokenType::_bool:{
            index++;
            checkIfLastToken("Expected an identifier name after `bool` token");

            if(getNextToken() != TokenType::ident){
                Log::Error("Expected an identifier name after `bool` token at " + getNextTokenPos());
                exit(1);
            }

            auto ident = m_allocator.alloc<Node::Ident>();
            ident->value = tokens.at(index).value.value();

            index++;
            checkIfLastToken("Expected a `;` to declare a boolean variable and end the line or an expression for declaring it");

            if(getNextToken() == TokenType::semi){
                auto stmt = m_allocator.alloc<Node::Stmt>();
                auto var = m_allocator.alloc<Node::Variable>();
                var->ident = ident;
                var->type = VarType::_bool;
                stmt->stmt = var;
                return stmt;
            }
            else if(getNextToken() == TokenType::equal){
                index++;
                checkIfLastToken("Expected an expression after token `=`");

                auto bool_expr = parseBoolExpr();
                checkIfLastToken("Expected a `;` to end the declaration of the boolean variable");

                auto stmt = m_allocator.alloc<Node::Stmt>();
                auto var = m_allocator.alloc<Node::Variable>();
                auto expr = m_allocator.alloc<Node::Expr>();
                expr->expr = bool_expr;
                var->ident = ident;
                var->type = VarType::_bool;
                var->expr = expr;
                stmt->stmt = var;
                return stmt;
            }
            else{
                Log::Error("Expected a `;` to declare a boolean variable and end the line or an expression for declaring it at " +
                getNextTokenPos());
                exit(1);
            }
        }

            /// HASH COMMANDS
        case TokenType::hash: {
            index++;
            checkIfLastToken("Expected an action name after the `#`");

            if (getNextToken() == TokenType::link) {
                index++;
                checkIfLastToken("Expected a string for a link file but reached the end of file");

                if (getNextToken() == TokenType::lit_string) {
                    index++;
                    if (getNextToken(true) == TokenType::new_line) {
                        auto link = m_allocator.alloc<Node::Link>();
                        auto s = m_allocator.alloc<Node::LitString>();
                        s->value = tokens.at(index - 1).value.value();
                        link->value = s;

                        auto stmt = m_allocator.alloc<Node::Stmt>();
                        stmt->stmt = link;
                        return stmt;
                    } else {
                        Log::Error("Expected a new line after `#link` statement at " + getNextTokenPos());
                        exit(1);
                    }
                } else {
                    Log::Error("Expected the link file as a literal string at " + getNextTokenPos());
                    exit(1);
                }
            } else if (getNextToken() == TokenType::_extern) {
                index++;
                checkIfLastToken("Expected an identifier name to import externally");
                if (getNextToken() != TokenType::ident) {
                    Log::Error("Expected an identifier name to import externally at " + getNextTokenPos());
                    exit(1);
                }

                auto string = m_allocator.alloc<Node::LitString>();
                string->value = tokens.at(index).value.value();

                index++;
                if (getNextToken(true) != TokenType::new_line) {
                    Log::Error("Expected a new line after #extern command but got an unexpected token at " +
                               getNextTokenPos());
                    exit(1);
                }

                auto assembly = m_allocator.alloc<Node::Assembly>();
                assembly->code = string;
                assembly->section = Node::Asm_Section::external;

                auto stmt = m_allocator.alloc<Node::Stmt>();
                stmt->stmt = assembly;

                return stmt;
            } else {
                Log::Info("hash command not implemented");
            }
        }

            /// SCOPES
        case TokenType::scope_open: {
            checkIfLastToken("Un-closed scope, expected `}`");

            auto scope = m_allocator.alloc<Node::Scope>();
            index++;
            while (auto stmt = parseStmt()) {
                scope->stmts.emplace_back(stmt.value());
                index++;
            }

            if (getNextToken() == TokenType::scope_close) {
                auto stmt = m_allocator.alloc<Node::Stmt>();
                stmt->stmt = scope;
                return stmt;
            }

            Log::Error("Un-closed scope, expected `}` at " + getNextTokenPos());
            exit(1);
        }

            /// IF
        case TokenType::_if: {
            checkIfLastToken("Expected `(` after if keyword with an expr");
            index++;

            if (getNextToken() != TokenType::expr_open) {
                Log::Error("Expected an `(` after if keyword at " + getNextTokenPos());
                exit(1);
            }
            index++;

            auto _if = m_allocator.alloc<Node::If>();
            auto expr = parseBoolExpr();

            if (getNextToken() != TokenType::expr_close) {
                Log::Error("Expected `)` at the end of the if condition at " + getNextTokenPos());
                exit(1);
            }
            index++;

            auto stmt = parseStmt();

            if(!stmt.has_value()){
                Log::Error("Expected a statement or scope to end the if statement at " + getNextTokenPos());
                exit(1);
            }

            if (!std::holds_alternative<Node::Scope *>(stmt.value()->stmt)) {
                auto scope = m_allocator.alloc<Node::Scope>();
                scope->stmts.emplace_back(stmt.value());
                _if->stmt = scope;
            } else {
                _if->stmt = std::get<Node::Scope *>(stmt.value()->stmt);
            }

            _if->expr = expr;

            auto ret_stmt = m_allocator.alloc<Node::Stmt>();
            ret_stmt->stmt = _if;

            return ret_stmt;
        }

            /// ELSE AND ELSE IF
        case TokenType::_else: {
            index++;
            checkIfLastToken("Expected a statement after keyword else");

            /// Else if
            if (getNextToken() == TokenType::_if) {
                index++;
                checkIfLastToken("Expected a condition after the else if statement");

                if (getNextToken() != TokenType::expr_open) {
                    Log::Error("Expected an `(` after else if at " + getNextTokenPos());
                    exit(1);
                }
                index++;
                checkIfLastToken("Expected a condition to check after `else if(`");

                auto expr = parseBoolExpr();

                if (getNextToken() != TokenType::expr_close) {
                    Log::Error("Expected `)` at the end of the else if condition at " + getNextTokenPos());
                    exit(1);
                }
                index++;
                checkIfLastToken("Expected a statement to execute after the else if condition");


                auto stmt = parseStmt(); // guaranteed a value
                auto scope = m_allocator.alloc<Node::Scope>();

                if(!stmt.has_value()){
                    Log::Error("Expected a statement or scope to end the else if statement at " + getNextTokenPos());
                    exit(1);
                }

                if (!std::holds_alternative<Node::Scope *>(stmt.value()->stmt))
                    scope->stmts.emplace_back(stmt.value());
                else
                    scope = std::get<Node::Scope *>(stmt.value()->stmt);

                auto else_stmt = m_allocator.alloc<Node::Elif>();
                else_stmt->stmt = scope;
                else_stmt->expr = expr;

                auto return_stmt = m_allocator.alloc<Node::Stmt>();
                return_stmt->stmt = else_stmt;
                return return_stmt;
            }

            /// Else
            auto stmt = parseStmt(); // guaranteed a value
            auto scope = m_allocator.alloc<Node::Scope>();

            if(!stmt.has_value()){
                Log::Error("Expected a statement or scope to end the else statement at " + getNextTokenPos());
                exit(1);
            }

            if (!std::holds_alternative<Node::Scope *>(stmt.value()->stmt))
                scope->stmts.emplace_back(stmt.value());
            else
                scope = std::get<Node::Scope *>(stmt.value()->stmt);

            auto else_stmt = m_allocator.alloc<Node::Else>();
            else_stmt->stmt = scope;

            auto return_stmt = m_allocator.alloc<Node::Stmt>();
            return_stmt->stmt = else_stmt;
            return return_stmt;
        }

        /// WHILE LOOPS
        case TokenType::_while: {
            auto stmt = m_allocator.alloc<Node::Stmt>();
            auto _while = m_allocator.alloc<Node::While>();

            index++;
            checkIfLastToken("Expected a condition after token `while`");

            if(getNextToken() != TokenType::expr_open){
                Log::Error("Expected `(` followed up with with a condition at " + getNextTokenPos());
                exit(1);
            }
            index++;
            checkIfLastToken("Expected a condition after `while(`");

            auto expr = parseBoolExpr();
            _while->expr = expr;

            if(getNextToken() != TokenType::expr_close){
                Log::Error("Expected an `)` to close-off the condition at " + getNextTokenPos());
                exit(1);
            }
            index++;
            checkIfLastToken("Expected a statment or scope after the while statement");

            if(getNextToken() == TokenType::semi){
                stmt->stmt = _while;
                return stmt;
            }

            auto scope = parseStmt();

            if(!scope.has_value()){
                Log::Error("Expected a statment or scope after the while statement at " + getNextTokenPos());
                exit(1);
            }

            if(std::holds_alternative<Node::Scope*>(scope.value()->stmt)){
                _while->scope = std::get<Node::Scope*>(scope.value()->stmt);
            }
            else{
                auto wrapped_stmt = m_allocator.alloc<Node::Scope>();
                wrapped_stmt->stmts.emplace_back(scope.value());
                _while->scope = wrapped_stmt;
            }

            stmt->stmt = _while;
            return stmt;
        }

            /// REASSIGNMENT
        case TokenType::ident: {
            std::string identValue = tokens.at(index).value.value();
            index++;
            checkIfLastToken(("Expected something after identifier" + identValue).c_str());

            Node::Reassign *stmt = m_allocator.alloc<Node::Reassign>();
            Node::Ident *ident = m_allocator.alloc<Node::Ident>();
            ident->value = identValue;
            stmt->ident = ident;

            TokenType nextToken = getNextToken();
            switch (nextToken) {
                case TokenType::plus:
                    index++;
                    if (getNextToken() == TokenType::plus) {
                        // Parse ident++
                        index++;
                        Node::BinExpr *binExpr = m_allocator.alloc<Node::BinExpr>();
                        Node::BinExprAdd *binExprAdd = m_allocator.alloc<Node::BinExprAdd>();
                        Node::IntExpr *lhs = m_allocator.alloc<Node::IntExpr>();
                        Node::IntExpr *rhs = m_allocator.alloc<Node::IntExpr>();
                        Node::Term *lhsTerm = m_allocator.alloc<Node::Term>();
                        Node::Term *rhsTerm = m_allocator.alloc<Node::Term>();
                        Node::LitInt *rhsInt = m_allocator.alloc<Node::LitInt>();
                        rhsInt->value = "1";

                        lhsTerm->term = ident;
                        rhsTerm->term = rhsInt;
                        lhs->var = lhsTerm;
                        rhs->var = rhsTerm;
                        binExprAdd->lhs = lhs;
                        binExprAdd->rhs = rhs;
                        binExpr->expr = binExprAdd;

                        Node::IntExpr *expr = m_allocator.alloc<Node::IntExpr>();
                        expr->var = binExpr;
                        stmt->expr = expr;
                    } else if (getNextToken() == TokenType::equal) {
                        // Parse ident += expr
                        index++;
                        Node::BinExpr *binExpr = m_allocator.alloc<Node::BinExpr>();
                        Node::BinExprAdd *binExprAdd = m_allocator.alloc<Node::BinExprAdd>();
                        Node::IntExpr *lhs = m_allocator.alloc<Node::IntExpr>();
                        Node::Term *lhsTerm = m_allocator.alloc<Node::Term>();
                        lhsTerm->term = ident;
                        lhs->var = lhsTerm;
                        Node::IntExpr *rhs = parseIntExpr();
                        binExprAdd->lhs = lhs;
                        binExprAdd->rhs = rhs;
                        binExpr->expr = binExprAdd;

                        Node::IntExpr *expr = m_allocator.alloc<Node::IntExpr>();
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
                        Node::BinExpr *binExpr = m_allocator.alloc<Node::BinExpr>();
                        Node::BinExprSub *binExprSub = m_allocator.alloc<Node::BinExprSub>();
                        Node::IntExpr *lhs = m_allocator.alloc<Node::IntExpr>();
                        Node::IntExpr *rhs = m_allocator.alloc<Node::IntExpr>();
                        Node::Term *lhsTerm = m_allocator.alloc<Node::Term>();
                        Node::Term *rhsTerm = m_allocator.alloc<Node::Term>();
                        Node::LitInt *rhsInt = m_allocator.alloc<Node::LitInt>();
                        rhsInt->value = "1";

                        lhsTerm->term = ident;
                        rhsTerm->term = rhsInt;
                        lhs->var = lhsTerm;
                        rhs->var = rhsTerm;
                        binExprSub->lhs = lhs;
                        binExprSub->rhs = rhs;
                        binExpr->expr = binExprSub;

                        Node::IntExpr *expr = m_allocator.alloc<Node::IntExpr>();
                        expr->var = binExpr;
                        stmt->expr = expr;
                    } else if (getNextToken() == TokenType::equal) {
                        // Parse ident -= expr
                        index++;
                        Node::BinExpr *binExpr = m_allocator.alloc<Node::BinExpr>();
                        Node::BinExprSub *binExprSub = m_allocator.alloc<Node::BinExprSub>();
                        Node::IntExpr *lhs = m_allocator.alloc<Node::IntExpr>();
                        Node::Term *lhsTerm = m_allocator.alloc<Node::Term>();
                        lhsTerm->term = ident;
                        lhs->var = lhsTerm;
                        Node::IntExpr *rhs = parseIntExpr();
                        binExprSub->lhs = lhs;
                        binExprSub->rhs = rhs;
                        binExpr->expr = binExprSub;

                        Node::IntExpr *expr = m_allocator.alloc<Node::IntExpr>();
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
                        Node::BinExpr *binExpr = m_allocator.alloc<Node::BinExpr>();
                        Node::BinExprMul *binExprMul = m_allocator.alloc<Node::BinExprMul>();
                        Node::IntExpr *lhs = m_allocator.alloc<Node::IntExpr>();
                        Node::Term *lhsTerm = m_allocator.alloc<Node::Term>();
                        lhsTerm->term = ident;
                        lhs->var = lhsTerm;
                        Node::IntExpr *rhs = parseIntExpr();
                        binExprMul->lhs = lhs;
                        binExprMul->rhs = rhs;
                        binExpr->expr = binExprMul;

                        Node::IntExpr *expr = m_allocator.alloc<Node::IntExpr>();
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
                        Node::BinExpr *binExpr = m_allocator.alloc<Node::BinExpr>();
                        Node::BinExprDiv *binExprDiv = m_allocator.alloc<Node::BinExprDiv>();
                        Node::IntExpr *lhs = m_allocator.alloc<Node::IntExpr>();
                        Node::Term *lhsTerm = m_allocator.alloc<Node::Term>();
                        lhsTerm->term = ident;
                        lhs->var = lhsTerm;
                        Node::IntExpr *rhs = parseIntExpr();
                        binExprDiv->lhs = lhs;
                        binExprDiv->rhs = rhs;
                        binExpr->expr = binExprDiv;

                        Node::IntExpr *expr = m_allocator.alloc<Node::IntExpr>();
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
                        Node::BinExpr *binExpr = m_allocator.alloc<Node::BinExpr>();
                        Node::BinExprMod *binExprMod = m_allocator.alloc<Node::BinExprMod>();
                        Node::IntExpr *lhs = m_allocator.alloc<Node::IntExpr>();
                        Node::Term *lhsTerm = m_allocator.alloc<Node::Term>();
                        lhsTerm->term = ident;
                        lhs->var = lhsTerm;
                        Node::IntExpr *rhs = parseIntExpr();
                        binExprMod->lhs = lhs;
                        binExprMod->rhs = rhs;
                        binExpr->expr = binExprMod;

                        Node::IntExpr *expr = m_allocator.alloc<Node::IntExpr>();
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

            Node::Stmt *returnStmt = m_allocator.alloc<Node::Stmt>();
            returnStmt->stmt = stmt;
            return returnStmt;
        }

            /// ASSEMBLY INSIDE THE PROGRAM
            /// has instructions for modifying assembly directly from the program
            /// ASM .text, .data, .bss and external functions
        case TokenType::_asm_text:
        case TokenType::_asm_data:
        case TokenType::_asm_bss: {

            Node::Asm_Section section;

            switch (getNextToken()) {
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
            if (getNextToken() != TokenType::less_then) {
                Log::Error("Expected a `<` to stream into the output assembly at " + getNextTokenPos());
                exit(1);
            }

            index++;
            checkIfLastToken("Expected a string value to stream into the output assembly but reached the last token");
            if (getNextToken() != TokenType::lit_string) {
                Log::Error("Expected a string value to stream into the output assembly at " + getNextTokenPos());
                exit(1);
            }

            auto code = m_allocator.alloc<Node::LitString>();
            code->value = tokens.at(index).value.value();

            index++;
            if (getNextToken() != TokenType::semi) {
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
        case TokenType::new_line:
        case TokenType::scope_close:
            return {};

        /// UNEXPECTED TOKEN
        default:
            Log::Error("Unexpected token at " + getNextTokenPos());
            exit(1);
    }
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