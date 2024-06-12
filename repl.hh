#pragma once

//#include "interpreter.h"
#include "backends/import.hh"
#include "scanner.hh"
#include "vm.hh"
#include <string>

class REPL
{
public:
    REPL();
    static void start();
    static void startDevMode();

private:
    std::unique_ptr<StackBackend> backend;
    std::unique_ptr<VM> vm;
    std::vector<Instruction> bytecode;
    static std::string readInput();
    static std::string readFile(const std::string& filename);
    static void debug(Scanner scanner, Parser parser);
};
