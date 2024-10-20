#ifndef REGISTER_BACKEND_HH
#define REGISTER_BACKEND_HH

#include "backend.hh"
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <variant>
#include <vector>

class RegisterBackend : public Backend {
public:
    explicit RegisterBackend(std::vector<Instruction> &program)
        : program(program) {}

    void execute(const Instruction &instruction) override;
    void dumpRegisters() override;
    void run(const std::vector<Instruction> &program) override;

private:
    unsigned int pc = 0;            // program counter
    std::vector<ValuePtr> registers; // Registers for storing data
    std::vector<ValuePtr> constants; // Constants for storing data
    std::vector<ValuePtr> variables; // Variables for storing data location
    std::vector<Instruction> &program;
    std::vector<std::thread> threads;
    std::mutex mtx; // Mutex for synchronization
  //  size_t pc = 0;

    void performUnaryOperation(int op);
    void performBinaryOperation(int op);
    void performLogicalOperation(int op);
    void performComparisonOperation(int op);
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

#endif // REGISTER_BACKEND_HH
