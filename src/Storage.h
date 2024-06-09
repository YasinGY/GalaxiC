#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <cstdint>

#include "Node.h"
#include "Log.h"
#include "Variable.h"

class Storage{
public:
    void StoreVariable(const std::string& ident, bool init, VarType type);
    bool IdentExists(const std::string& ident);
    bool IsIdentInit(const std::string& ident);
    size_t GetStackPosition(const std::string& ident);
    inline uint64_t GetStackSize() { return stack_size; }

private:

    struct Variable{
        bool init;
        std::string ident;
        size_t size;
    };

    std::vector<Variable> variables;
    uint64_t stack_size;
};