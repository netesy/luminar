#pragma once

#include "backends/import.hh"
#include "scanner.hh"
#include "vm.hh"
#include <optional>
#include <variant>
#include <string>
#include <memory>

class REPL
{
public:
    REPL();
    ~REPL();
    
    void start(const std::string &filename = "");
    void startDevMode(const std::string &filename = "");
    void run(const std::string &input, const std::string &filename = "", const std::string &filepath = "");

private:
    // Core components
    std::unique_ptr<MemoryManager<>> memoryManager;
    std::unique_ptr<MemoryManager<>::Region> region;
    std::shared_ptr<TypeSystem> typeSystem;
    std::unique_ptr<Functions> functions;
    
    // Execution state
    std::vector<Instruction> bytecode;
    std::unique_ptr<Algorithm> parser;
    std::unique_ptr<StackBackend> backend;
    std::unique_ptr<VM> vm;

    // Helper methods
    std::string readInput() const;
    std::string readFile(const std::string &filename) const;
    void debug(const Scanner &scanner, const Algorithm &parser) const;
    void initializeVM();
    void cleanup();
    void resetExecutionState();
};
