CXX := g++
CXXFLAGS := -std=c++17 -I.
TARGET := my_compiler

SOURCES := \
        main.cpp \
        Compiler/Lexer.cpp \
        Compiler/Parser.cpp \
        Compiler/CodeGenerator.cpp \
        Compiler/SymbolTable.cpp \
        Linker/Linker.cpp \
        Vm/VirtualMachine.cpp \
        Vm/Loader.cpp \
        src/frontend/FrontendPipeline.cpp \
        src/backend/BackendPipeline.cpp \
        src/linker/ExecutableFormat.cpp \
        src/runtime/VmMonitor.cpp

all: build

build:
        $(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)
