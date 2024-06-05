#ifndef STACK_HH
#define STACK_HH

#include "backend.hh"
#include <iostream>
#include <stack>
#include <variant>
#include <vector>
#include <string>


class StackBackend : public Backend {
public:
    explicit StackBackend(std::vector<Instruction> &program)
        : program(program) {}

    void execute(const Instruction &instruction) override;
    void dumpRegisters() override;

private:
    unsigned int pc = 0;            // program counter
    std::stack<Value> stack;        // Stack for storing data
    std::vector<Value> constants;   // Constants for storing data
    std::vector<Value> variables;   // Variables for storing data location
    std::vector<Instruction> &program;
    std::vector<std::thread> threads;
    std::mutex mtx; // Mutex for synchronization

    void performUnaryOperation(const Instruction& instruction);
    void performBinaryOperation(const Instruction& instruction);
    void performComparisonOperation(const Instruction& instruction);
    void performLogicalOperation(const Instruction& instruction);
    
    void handleLoadConst(unsigned int constantIndex);
    void handleDeclareVariable(unsigned int variableIndex);
    void handleLoadVariable(unsigned int variableIndex);
    void handleStoreVariable(unsigned int variableIndex);
    void handleParallel(unsigned int taskCount);
    void handleConcurrent(unsigned int taskCount);
    void handlePrint();
    void handleHalt();
    void handleWhileLoop();
    void handleInput();
    void handleOutput();

    //helpers functions
    void concurrent(std::vector<std::function<void()>> tasks);
};

#endif // STACK_BACKEND_HH
