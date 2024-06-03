#ifndef VM_HH
#define VM_HH

#include "opcodes.hh"
#include "parser.hh"
#include <iostream>
#include <variant>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>

using Value = std::variant<int32_t, double, bool, std::string>;

class RegisterVM {
public:
    explicit RegisterVM(Parser &parser)
        : parser(parser)
    {
        program = parser.getBytecode();
    }

    void run();
    void execute(const Instruction &instruction);
    void dumpRegisters();

private:
    Parser &parser;
    unsigned int pc = 0;            // program counter
    std::vector<Value> registers;   // Registers for storing data
    std::vector<Value> constants;   // Constants for storing data
    std::vector<unsigned int, Value> variables; // Variables for storing data location
    std::vector<Instruction> program;
    std::vector<std::thread> threads;
    std::mutex mtx; // Mutex for synchronization
  //  size_t pc = 0;

    void performUnaryOperation(int op);
    void performBinaryOperation(int op);
    void performLogicalOperation(int op);
    void performComparisonOperation(int op);
    void handleLoadConst(unsigned int constantIndex);
    //void handleStoreValue(unsigned int storeIndex);
    void handleDeclareVariable(unsigned int variableIndex);
    void handleLoadVariable(unsigned int variableIndex);
    void handleStoreVariable(unsigned int variableIndex);
    void handleParallel(unsigned int taskCount);
    void handleConcurrent(unsigned int taskCount);
    void concurrent(std::vector<std::function<void()>> tasks);
    void handlePrint();
    void handleHalt();
    void handleInput();
    void handleOutput();
};

#endif // VM_HH
