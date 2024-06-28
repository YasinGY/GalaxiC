cmake_minimum_required(VERSION 3.28)
project(GalaxiC)

set(CMAKE_CXX_STANDARD 17)

add_executable(GalaxiC src/main.cpp
        src/Tokenizer.h
        src/Tokenizer.cpp
        src/Arena.h
        src/Token.h
        src/Node.h
        src/Generator.h
        src/Storage.h
        src/Storage.cpp
        src/Parser.h
        src/Parser.cpp
        src/Core.h
        src/Log.h
        src/Generator.cpp
        src/Assemble.h
        src/Assemble.cpp
        src/Variable.h
        src/Labels.h
)
