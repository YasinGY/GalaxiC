#include "Parser.h"

Node::Term* Parser::parseTerm()  {
    if(tokens.at(index).type != TokenType::ident && tokens.at(index).type != TokenType::lit_int){
        Log::Error("Expected a term (identifier or integer literal)");
        exit(1);
    }
    Node::Term* term = m_allocator.alloc<Node::Term>();

    if(tokens.at(index).type == TokenType::ident){
        Node::Ident* id = m_allocator.alloc<Node::Ident>();
        id->value = tokens.at(index).value.value();
        term->term = id;
    }
    else{
        Node::LitInt* litInt = m_allocator.alloc<Node::LitInt>();
        litInt->value = tokens.at(index).value.value();
        term->term = litInt;
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
        if (index >= tokens.size() || !isBinOp(tokens.at(index).type))
            break;

        checkIfLastToken("Expected a binary operator!");

        TokenType op = tokens.at(index).type;
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
            // TODO: Handle the modulo operator when implemented
            Log::Info("Feature not implemented yet!");
            exit(1);
        } else {
            Log::Error("Failed to parse the int expression!");
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
        if (newline)
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

/// public

Node::Program *Parser::parse() {
    for(index = 0; index < tokens.size(); index++){
        if(getNextToken() == TokenType::exit){
            checkIfLastToken("Expected an expression after exit token");
            index++;

            if(getNextToken() == TokenType::expr_open){
                checkIfLastToken("Expected an exit code after exit(");
                index++;

                Node::Expr* expr = parseIntExpr();
                if(getNextToken() == TokenType::expr_close){
                    checkIfLastToken("Expected \';\' after exit statement");
                    index++;
                    if(getNextToken() == TokenType::semi){
                        Node::Exit* e = m_allocator.alloc<Node::Exit>();
                        e->expr = expr;
                        program.prg.emplace_back(Node::Stmt{e});
                        continue;
                    }
                }
            }
        }

        else if(getNextToken() == TokenType::_int16 || getNextToken() == TokenType::_int32 ||
                getNextToken() == TokenType::_int64){

            int bits = (getNextToken() == TokenType::_int16) ? 16 : (getNextToken() == TokenType::_int32) ? 32 : 64;
            index++;
            checkIfLastToken("Expected an identifier after the \'int\' keyword for variable declaration");

            if(getNextToken() == TokenType::ident){
                Node::Ident* ident = m_allocator.alloc<Node::Ident>();
                ident->value = tokens.at(index).value.value();
                index++;
                checkIfLastToken("Expected a \';\' or initialization of the ident");

                if(getNextToken() == TokenType::semi){
                    index++;
                    auto var = m_allocator.alloc<Node::Variable>();
                    auto term = m_allocator.alloc<Node::Term>();
                    auto value = m_allocator.alloc<Node::Ident>();
                    value->value = "";
                    term->term = value;

                    if(bits == 16)
                        var->type = VarType::_int16;
                    else if(bits == 32)
                        var->type = VarType::_int32;
                    else
                        var->type = VarType::_int64;

                    var->expr->var = term;
                    program.prg.emplace_back(Node::Stmt{var});
                    continue;
                }
                else if(getNextToken() == TokenType::equal){
                    index++;
                    checkIfLastToken(std::string("Expected an int value to give to identifier \'" + ident->value + "\'").c_str());

                    auto expr = parseIntExpr();
                    if(getNextToken() == TokenType::semi){
                        auto var = m_allocator.alloc<Node::Variable>();
                        var->type = VarType::_int32;
                        var->expr = expr;
                        program.prg.emplace_back(Node::Stmt{var});
                        continue;
                    }
                    else{
                        Log::Error("Expected a \';\' after declaring an int variable");
                        exit(1);
                    }
                }
                else{
                    Log::Error("Expected a \'=\' or \';\' after giving the name of the identifier to finish the statement");
                    exit(1);
                }
            }
            else{
                Log::Error("Expected an identifier to declare an int variable");
                exit(1);
            }
        }

        else if(getNextToken() == TokenType::hash){
            index++;
            checkIfLastToken("Expected an action name after the \'#\'");

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
                        program.prg.emplace_back(Node::Stmt{link});
                        continue;
                    }
                    else{
                        Log::Error("Expected a new line after \'#link\' statement");
                        exit(1);
                    }
                }
                else{
                    Log::Error("Expected the link file as a literal string");
                    exit(1);
                }
            }
            else{
                // TODO - hash commands
            }
        }
    }

    return &program;
}