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
                    TokenType type = TokenDict[buffer];
                    tokens.emplace_back(Token{type});
                }
                else{
                    tokens.emplace_back(Token{TokenType::ident, buffer});
                }
                buffer.clear();
            }

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
                    if(isdigit(code.at(i + 1)) && !isTokenInt(tokens.end()->type)){
                        // check for negative literal ints
                        uint64_t position = i;
                        position++;
                        std::string buff = "";
                        while(true){
                            if(isCharTokenBreaker(code.at(position))){
                                if(isStringInteger(buff)){
                                    tokens.emplace_back(Token{TokenType::lit_int, '-' + buff});
                                    i = position - 1;
                                    break;
                                }
                                else{
                                    tokens.emplace_back(Token{TokenType::minus});
                                    break;
                                }
                            }
                            buff += code.at(position);
                            position++;
                        }
                    }
                    else {
                        tokens.emplace_back(Token{TokenType::minus});
                    }
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
                    tokens.emplace_back(Token{TokenType::less_then});
                    break;
                case '>':
                    tokens.emplace_back(Token{TokenType::greater_then});
                    break;
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

bool Tokenizer::isTokenInt(TokenType type) {
    // will add more when doubles and floats get added
    switch(type){
        case TokenType::lit_int:
        case TokenType::ident:
            return true;
        default:
            return false;
    }
}

void Tokenizer::removeComments() {
    std::string result;
    bool inMultilineComment = false;
    bool inSingleQuote = false;
    bool inDoubleQuote = false;

    for (uint64_t i = 0; i < code.length(); ++i) {
        if (!inMultilineComment && !inSingleQuote && !inDoubleQuote && code[i] == '/') {
            if (i + 1 < code.length() && code[i + 1] == '/') {
                // Single-line comment
                i += 2;
                while (i < code.length() && code[i] != '\n') {
                    ++i;
                }
                result += '\n'; // Add the newline character after the single-line comment
            } else if (i + 1 < code.length() && code[i + 1] == '*') {
                // Multi-line comment start
                inMultilineComment = true;
                i += 2;
            } else {
                result += code[i];
            }
        } else if (inMultilineComment && code[i] == '*' && i + 1 < code.length() && code[i + 1] == '/') {
            // Multi-line comment end
            inMultilineComment = false;
            i += 2;
        } else if (!inMultilineComment && code[i] == '\'') {
            // Single quote
            inSingleQuote = !inSingleQuote;
            result += code[i];
        } else if (!inMultilineComment && code[i] == '"') {
            // Double quote
            inDoubleQuote = !inDoubleQuote;
            result += code[i];
        } else if (!inMultilineComment && !inSingleQuote && !inDoubleQuote) {
            result += code[i];
        } else {
            // Inside a multi-line comment, single quote, or double quote, keep the characters
            result += code[i];
        }
    }

    code = result;
}