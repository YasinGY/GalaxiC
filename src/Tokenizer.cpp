#include "Tokenizer.h"

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    std::string buffer;
    size_t column = 0;
    size_t line = 1;
    size_t last_buffer_col;

    for (size_t i = 0; i < code.length(); i++) {
        char c = code.at(i);

        if (c == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }

        if (c == '\"') {
            std::string value;
            size_t string_start_col = column; // Record starting column of the string literal
            while (true) {
                i++;
                if (i >= code.length()) {
                    std::stringstream msg;
                    msg << "Expected a closing quotation mark before end of the file at ";
                    msg << line << ':' << string_start_col;
                    Log::Error(msg.str());
                    exit(1);
                }
                c = code.at(i);
                if (c == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                if (c == '\"') {
                    tokens.emplace_back(Token{TokenType::lit_string, value, line, string_start_col});
                    break;
                } else if (c == '\n') {
                    std::stringstream msg;
                    msg << "Expected a closing quotation mark before end of the line at ";
                    msg << line << ':' << column;
                    Log::Error(msg.str());
                    exit(1);
                } else {
                    value += c;
                }
            }
        }

        if (isCharTokenBreaker(c)) {
            if (!buffer.empty()) {
                if (isStringInteger(buffer)) {
                    tokens.emplace_back(Token{TokenType::lit_int, buffer, line, last_buffer_col});
                } else if (isInTokenDict(buffer)) {
                    TokenType type = TokenDict[buffer];
                    tokens.emplace_back(Token{type, buffer, line, last_buffer_col});
                } else {
                    tokens.emplace_back(Token{TokenType::ident, buffer, line, last_buffer_col});
                }
                buffer.clear();
            }

            switch (c) {
                case '\n':
                case '\0':
                    tokens.emplace_back(Token{TokenType::new_line, {}, line, column});
                    break;
                case ';':
                    tokens.emplace_back(Token{TokenType::semi, {}, line, column});
                    break;
                case '(':
                    tokens.emplace_back(Token{TokenType::expr_open, {}, line, column});
                    break;
                case ')':
                    tokens.emplace_back(Token{TokenType::expr_close, {}, line, column});
                    break;
                case '{':
                    tokens.emplace_back(Token{TokenType::scope_open, {}, line, column});
                    break;
                case '}':
                    tokens.emplace_back(Token{TokenType::scope_close, {}, line, column});
                    break;
                case '+':
                    tokens.emplace_back(Token{TokenType::plus, {}, line, column});
                    break;
                case '-':
                    if (isdigit(code.at(i + 1)) && !isTokenInt(tokens.end()->type)) {
                        // check for negative literal ints
                        size_t position = i;
                        position++;
                        std::string buff = "";
                        while (true) {
                            if (isCharTokenBreaker(code.at(position))) {
                                if (isStringInteger(buff)) {
                                    tokens.emplace_back(Token{TokenType::lit_int, '-' + buff, line, column});
                                    i = position - 1;
                                    column++;
                                    break;
                                } else {
                                    tokens.emplace_back(Token{TokenType::minus, {}, line, column});
                                    column++;
                                    break;
                                }
                            }
                            buff += code.at(position);
                            position++;
                            column++;
                        }
                    } else {
                        tokens.emplace_back(Token{TokenType::minus, {}, line, column});
                    }
                    break;
                case '*':
                    tokens.emplace_back(Token{TokenType::star, {}, line, column});
                    break;
                case '/':
                    tokens.emplace_back(Token{TokenType::slash, {}, line, column});
                    break;
                case '=':
                    tokens.emplace_back(Token{TokenType::equal, {}, line, column});
                    break;
                case '!':
                    tokens.emplace_back(Token{TokenType::_not, {}, line, column});
                    break;
                case '#':
                    tokens.emplace_back(Token{TokenType::hash, {}, line, column});
                    break;
                case '%':
                    tokens.emplace_back(Token{TokenType::percent, {}, line, column});
                    break;
                case '&':
                    tokens.emplace_back(Token{TokenType::_and, {}, line, column});
                    break;
                case '|':
                    tokens.emplace_back(Token{TokenType::_or, {}, line, column});
                    break;
                case '?':
                    tokens.emplace_back(Token{TokenType::qmark, {}, line, column});
                    break;
                case ':':
                    tokens.emplace_back(Token{TokenType::colon, {}, line, column});
                    break;
                case '.':
                    tokens.emplace_back(Token{TokenType::dot, {}, line, column});
                    break;
                case ',':
                    tokens.emplace_back(Token{TokenType::coma, {}, line, column});
                    break;
                case '<':
                    tokens.emplace_back(Token{TokenType::less_then, {}, line, column});
                    break;
                case '>':
                    tokens.emplace_back(Token{TokenType::greater_then, {}, line, column});
                    break;
            }
            last_buffer_col = column;
        } else {
            if (buffer.empty()) {
                last_buffer_col = column;
            }
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
