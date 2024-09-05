#pragma once

//#include "interpreter.h"
#include "backends/import.hh"
#include "scanner.hh"
#include "vm.hh"
#include <string>

class REPL
{
public:
    REPL(std::unique_ptr<Algorithm> parser);
    static void start(const std::string &filename);
    static void run(std::string input, const std::string &filename, const std::string &filepath);
    static void startDevMode(const std::string &filename);
    static void setParser(std::unique_ptr<Algorithm> newParser);

private:
    std::unique_ptr<StackBackend> backend;
    std::unique_ptr<VM> vm;
    std::unique_ptr<Algorithm> parser;
    std::vector<Instruction> bytecode;
    static std::string readInput();
    static std::string readFile(const std::string& filename);
    static void debug(const Scanner &scanner, const Algorithm &parser);
};
