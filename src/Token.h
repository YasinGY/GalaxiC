#pragma once

#include <string>
#include <optional>

enum class TokenType{
    exit, _if, _else, // keywords
    // data types, example: the keyword 'int'
    _int16, _int32, _int64, _string, _void, _let,
    // literal values like 1432 or "string value"
    lit_int, lit_string,
    // single char tokens
    semi, expr_open, expr_close, coma, colon, dot, _and, _not, qmark, stream,
    percent, hash, plus, minus, star, slash, equal, scope_open, scope_close,
    new_line,
    // others
    _import, define, link,
    // is for identifiers
    // is checked during parsing and generation
    ident,
};

struct Token{
    TokenType type;
    std::optional<std::string> value;
};
