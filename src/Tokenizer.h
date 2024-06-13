#pragma clang diagnostic push
#pragma ide diagnostic ignored "LocalValueEscapesScope"
#pragma once

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "Token.h"

class Tokenizer{
public:
    explicit Tokenizer(std::string  content) : code(std::move(content)) {}
    std::vector<Token> tokenize();

private:

    bool isCharTokenBreaker(const char& c);
    inline bool isInTokenDict(const std::string& str){ return TokenDict.find(str) != TokenDict.end(); }
    bool isStringInteger(const std::string& str);

    std::unordered_map<std::string, TokenType> TokenDict = {
            {"exit", TokenType::exit},
            {"let", TokenType::_let},
            {"int16", TokenType::_int16},
            {"short", TokenType::_int16},
            {"int", TokenType::_int32},
            {"int32", TokenType::_int32},
            {"int64", TokenType::_int64},
            {"long", TokenType::_int64},
            {"string", TokenType::_string},
            {"include", TokenType::_import},
            {"import", TokenType::_import},
            {"define", TokenType::define},
            {"link", TokenType::link},
            {"void", TokenType::_void},
            {"if", TokenType::_if},
            {"else", TokenType::_else},
    };

    char token_breakers[22] = {
            ' ', ';', '\n', '(', ')', '{', '}', '-', '*', '+', '=', '/', '#',
            '!', '%', '&', ':', '?', '.', ',', '\"', '\0'
    };

    std::string code;
};

#pragma clang diagnostic pop