# GalaxiC - Galaxi Compiler

A Compiler designed to be fast with a C/C++ like syntax and functionality, it is designed to combine the good features of C and good features from C++ and combine them together.
It currently supports:

exit(code); a built-in function that ends the program with the exit code you give it

shorts, int, long Variables. you can also use int16 instead of short, int32 instead of int and int64 instead of long which represents the size of the variables

asm_text, asm_bss and asm_data which is for directly inserting assembly code from the code it self, this helps with developing libraries for the compiler

extern is a keyword that loads in functions from assembly files which are linked to the program, the function name needs to be after the extern keyword in a string format

#link which is releated to the linking process, it is just like `extern` but loads in a file instead using the linker

if statements which are like in C

In development:

Booleans, they were added but was removed because of bugs and are going to be implemented again soon better than ever!
