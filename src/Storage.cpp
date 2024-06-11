#include "Storage.h"

void Storage::StoreVariable(const std::string& ident, bool init, VarType type) {
    Variable var;
    var.ident = ident;
    var.init = init;

    switch(type){
        case VarType::_char: // db
            var.size = 8;
            stack_size += 8;
            break;
        case VarType::_int16: // dw
            var.size = 16;
            stack_size += 16;
            break;
        case VarType::_float:
        case VarType::_int32: // dd
            var.size = 32;
            stack_size += 32;
            break;
        case VarType::_double:
        case VarType::_int64: // dq
            var.size = 64;
            stack_size += 64;
            break;
//        case VarType::_string: NOT IMPLEMENTED
//            var.size = value.length();
//            break;
    }
    variables.emplace_back(var);
}
bool Storage::IdentExists(const std::string &ident) {
    for(Variable var : variables)
        if(var.ident == ident)
            return true;
    return false;
}
bool Storage::IsIdentInit(const std::string &ident) {
    for(Variable var : variables) {
        if (var.ident != ident)
            continue;
        return var.init;
    }

    Log::Error("Ident name was not found in the variables in the isIdentInit function");
    exit(1);
}
size_t Storage::GetStackPosition(const std::string &ident) {
    size_t ret_value = stack_size;

    for(const Variable& var : variables) {
        ret_value -= var.size;

        if(var.ident == ident) {
            return ret_value;
        }
    }

    Log::Error("Ident was not found in the variables in function GetStackPosition");
    exit(1);
}