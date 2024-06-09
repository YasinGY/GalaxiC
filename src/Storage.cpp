#include "Storage.h"

void Storage::StoreVariable(const std::string& ident, const std::string& value, VarType type) {
    Variable var;
    var.ident = ident;
    var.init = value == "";

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

    for(uint64_t i = variables.size(); i > 0; i--){
        Variable var = variables.at(i - 1);

        if(var.ident == ident)
            return ret_value;

        ret_value -= var.size;
    }

    Log::Error("Ident was not found in the variables in function GetStackPosition");
    exit(1);
}