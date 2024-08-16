#pragma once

#include <iostream>

#define PLATFORM_WIN32      0
#define PLATFORM_WIN64      1
#define PLATFORM_LINUX32    2
#define PLATFORM_LINUX64    3

#define ASSERT(msg) { \
    std::cerr << "[ASSERTION] from function " << __FUNCTION__ \
              << " at line " << __LINE__ \
              << " in file " << __FILE__ << ", " \
              << msg << std::endl; \
    std::abort(); \
}

/*
 *      14/5: 505 lines - start (almost)!
 *      9/6:  1140 lines - binary expression, exit and (almost) int variables!
 *      10/6: 1146 lines - fixed int variables, 8/16/32/64 bit ints, char, short, int, long!
 *      11/6: 1178 lines - bug fixes and parentheses to binary expressions!
 *      12/6: 1247 lines - scopes!
 *      14/6: 1586 lines - reassignment and negative integers!
 *      15/6: 1756 lines - direct assembly modification!
 *      27/6: 2203 lines - booleans hopefully! not completed.
 *      31/7: 2394 lines - boolean variables and boolean parsing
 *      12/8: 2582 lines - fixing ard boolean parsing and bettering the labels and assembly generation
*/