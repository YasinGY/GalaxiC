#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <cstdint>

#include "Log.h"

enum class VarType{
    _int16, _int32, _int64, _float, _string, _double,
    _char,
};

class Storage{
public:
    void StoreVariable(const std::string& ident, const std::string& value, VarType type);
    bool IdentExists(const std::string& ident);
    bool IsIdentInit(const std::string& ident);
    size_t GetStackPosition(const std::string& ident);

private:

    struct Variable{
        bool init;
        std::string ident;
        size_t size;
    };

    std::vector<Variable> variables;
    uint64_t stack_size;
};