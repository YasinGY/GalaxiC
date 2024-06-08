#pragma once

#include <iostream>

class Log{
public:
    inline static void Debug(const std::string& msg){
        std::cout << "\033[92m" << msg << "\033[0m" << '\n';
    }
    inline static void Error(const std::string& msg){
        std::cout << "\033[91m" << msg << "\033[0m" << '\n';
    }
    inline static void Info(const std::string& msg){
        std::cout << "\033[96m" << msg << "\033[0m" << '\n';
    }
    inline static void Warning(const std::string& msg){
        std::cout << "\033[93m" << msg << "\033[0m" << '\n';
    }
};