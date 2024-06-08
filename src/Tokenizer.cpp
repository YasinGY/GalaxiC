#include "Tokenizer.h"


std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    std::string buffer;

    for(uint64_t i = 0; i < code.length(); i++){
        char c = code.at(i);

        if(c == '\"'){
            std::string value;
            while(true){
                i++;
                c = code.at(i);
                if(c == '\"'){
                    tokens.emplace_back(Token{TokenType::lit_string, value});
                    break;
                }
                else
                    value += c;
            }
        }

        if(isCharTokenBreaker(c)){
            if(!buffer.empty()){
                if(isStringInteger(buffer)){
                    tokens.emplace_back(Token{TokenType::lit_int, buffer});
                }
                else if(isInTokenDict(buffer)){
                    tokens.emplace_back(Token{TokenDict[buffer]});
                }
                else{
                    tokens.emplace_back(Token{TokenType::ident, buffer});
                }
            }
            buffer.clear();

            switch(c){
                case '\n':
                case '\0':
                    tokens.emplace_back(Token{TokenType::new_line});
                    break;
                case ';':
                    tokens.emplace_back(Token{TokenType::semi});
                    break;
                case '(':
                    tokens.emplace_back(Token{TokenType::expr_open});
                    break;
                case ')':
                    tokens.emplace_back(Token{TokenType::expr_close});
                    break;
                case '{':
                    tokens.emplace_back(Token{TokenType::scope_open});
                    break;
                case '}':
                    tokens.emplace_back(Token{TokenType::scope_close});
                    break;
                case '+':
                    tokens.emplace_back(Token{TokenType::plus});
                    break;
                case '-':
                    tokens.emplace_back(Token{TokenType::minus});
                    break;
                case '*':
                    tokens.emplace_back(Token{TokenType::star});
                    break;
                case '/':
                    tokens.emplace_back(Token{TokenType::slash});
                    break;
                case '=':
                    tokens.emplace_back(Token{TokenType::equal});
                    break;
                case '!':
                    tokens.emplace_back(Token{TokenType::_not});
                    break;
                case '#':
                    tokens.emplace_back(Token{TokenType::hash});
                    break;
                case '%':
                    tokens.emplace_back(Token{TokenType::percent});
                    break;
                case '&':
                    tokens.emplace_back(Token{TokenType::_and});
                    break;
                case '?':
                    tokens.emplace_back(Token{TokenType::qmark});
                    break;
                case ':':
                    tokens.emplace_back(Token{TokenType::colon});
                    break;
                case '.':
                    tokens.emplace_back(Token{TokenType::dot});
                    break;
                case ',':
                    tokens.emplace_back(Token{TokenType::coma});
                    break;
                case '<':
                    tokens.emplace_back(Token{TokenType::stream});
            }
        }
        else{
            buffer += c;
        }
    }

    return tokens;
}
bool Tokenizer::isCharTokenBreaker(const char& c){
    for(char token : token_breakers)
        if(token == c)
            return true;

    return false;
}
bool Tokenizer::isStringInteger(const std::string &str) {
    for(char c : str)
        if(!isdigit(c))
            return false;

    return true;
}