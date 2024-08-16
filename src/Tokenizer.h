#pragma clang diagnostic push
#pragma ide diagnostic ignored "LocalValueEscapesScope"
#pragma once

#include "PCH.h"
#include "Token.h"
#include "Log.h"

class Tokenizer{
public:
    inline explicit Tokenizer(std::string  content) : code(std::move(content)) {
        removeComments();
    }
    std::vector<Token> tokenize();

private:

    bool isCharTokenBreaker(const char& c);
    inline bool isInTokenDict(const std::string& str){ return TokenDict.find(str) != TokenDict.end(); }
    bool isStringInteger(const std::string& str);
    bool isTokenInt(TokenType type);
    void removeComments();

    std::unordered_map<std::string, TokenType> TokenDict = {
            {"exit", TokenType::exit},
            {"let", TokenType::_let},
            {"true", TokenType::_true},
            {"false", TokenType::_false},
            {"bool", TokenType::_bool},
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
            {"while", TokenType::_while},
            {"else", TokenType::_else},
            {"_asm_text", TokenType::_asm_text},
            {"_asm_data", TokenType::_asm_data},
            {"_asm_bss", TokenType::_asm_bss},
            {"extern", TokenType::_extern},
    };

    char token_breakers[25] = {
            ' ', ';', '\n', '(', ')', '{', '}', '-', '*', '+', '=',
            '/', '#','!', '%', '&', ':', '?', '.', ',', '\"', '|',
            '<', '>', '\0'
    };

    std::string code;
};

#pragma clang diagnostic pop