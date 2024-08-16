#ifndef VARIABLE_H
#define VARIABLE_H

#include "PCH.h"

enum class VarType{
    _char, _short, _int, _long, _bool
};

inline std::string VarTypeToString(const VarType& type){
    switch (type) {
        case VarType::_char:
            return "char";
        case VarType::_short:
            return "short";
        case VarType::_int:
            return "int";
        case VarType::_long:
            return "long";
        case VarType::_bool:
            return "bool";
    }

    exit(1);
}

#endif