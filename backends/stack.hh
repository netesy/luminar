#ifndef STACK_HH
#define STACK_HH

#include "backend.hh"
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <stack>
#include <string>
#include <thread>
#include <variant>
#include <vector>

class StackBackend : public Backend {
public:
    explicit StackBackend(std::vector<Instruction> &program)
        : program(program) {}

    void execute(const Instruction &instruction) override;
    void dumpRegisters() override;
    void run(const std::vector<Instruction> &program) override;

private:
    std::map<std::string, std::function<void()>> functions;
    std::stack<Value> stack;        // Stack for storing data
    std::vector<Value> constants;   // Constants for storing data
    std::vector<Value> variables;   // Variables for storing data location
    std::vector<Instruction> &program;
    std::vector<std::thread> threads;
    std::mutex mtx; // Mutex for synchronization
    unsigned int pc = 0;            // program counter

    void performUnaryOperation(const Instruction& instruction);
    void performBinaryOperation(const Instruction& instruction);
    void performComparisonOperation(const Instruction& instruction);
    void performLogicalOperation(const Instruction &instruction);

    void handleLoadConst(const Value &constantValue);
    void handleLoadStr(const std::string &constantValue);
    void handleDeclareVariable(unsigned int variableIndex);
    void handleLoadVariable(unsigned int variableIndex);
    void handleStoreVariable(unsigned int variableIndex);
    void handleDeclareFunction(const std::string& functionName);
    void handleCallFunction(const std::string& functionName);
    void handleParallel(unsigned int taskCount);
    void handleConcurrent(unsigned int taskCount);
    void handlePushArg(const Instruction &instruction);
    void handlePrint();
    void handleHalt();
    void handleJump();
    void handleJumpZero();
    void handleMatch();
    void handleInput();
    void handleOutput();

    //helpers functions
    void concurrent(std::vector<std::function<void()>> tasks);
    std::string getTypeName(const Value &value)
    {
        if (std::holds_alternative<int32_t>(value))
            return "int32_t";
        if (std::holds_alternative<double>(value))
            return "double";
        if (std::holds_alternative<bool>(value))
            return "bool";
        if (std::holds_alternative<std::string>(value))
            return "string";
        return "unknown";
    }
};

#endif // STACK_BACKEND_HH
