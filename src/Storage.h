#pragma once

#include "PCH.h"
#include "Node.h"
#include "Log.h"
#include "Variable.h"

class Storage{
public:
    void StoreVariable(const std::string& ident, bool init, VarType type);
    bool IdentExists(const std::string& ident);
    bool IsIdentInit(const std::string& ident);
    uint64_t GetStackPosition(const std::string& ident);
    inline uint64_t GetStackSize() { return stack_size; }
    void CreateScope();
    uint64_t EndScope(); // returns the stack size from last scope

private:

    struct Variable{
        bool init;
        std::string ident;
        size_t size;
    };
    struct Scope{
        uint64_t vars; // how many vars declared to pop of the variables vector
        uint64_t size;
    };

    std::vector<Scope> scopes;
    std::vector<Variable> variables;
    uint64_t stack_size = 0;
};