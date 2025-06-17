#pragma once

//#include "interpreter.h"
#include "backends/import.hh"
#include "scanner.hh"
#include "vm.hh"
#include <optional>
#include <variant>
#include <string>

class REPL
{
public:
    REPL();
    static void start(const std::string &filename);
    static void run(std::string input, const std::string &filename, const std::string &filepath);
    static void startDevMode(const std::string &filename);
    static void setParser(std::unique_ptr<Algorithm> newParser);

private:
    static MemoryManager<> memoryManager;
    static MemoryManager<>::Region region;
    static std::shared_ptr<TypeSystem> typeSystem;
    static Functions functions;
    static std::vector<Instruction> bytecode;
    static std::unique_ptr<Algorithm> parser;
    static std::unique_ptr<StackBackend> backend;
    static std::unique_ptr<VM> vm;

    static std::string readInput();
    static std::string readFile(const std::string& filename);
    static void debug(const Scanner &scanner, const Algorithm &parser);
};
