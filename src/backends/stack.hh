#ifndef STACK_HH
#define STACK_HH

#include "../types.hh"
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

class StackBackend : public Backend
{
public:
    explicit StackBackend(std::vector<Instruction> &program)
        : program(program)
    {}

    void run(const std::vector<Instruction> &program) override;
    void execute(const Instruction &instruction) override;
    void dumpRegisters() override;

private:
    std::stack<ValuePtr> stack;
    std::vector<ValuePtr> constants;
    std::vector<ValuePtr> variables;
    std::map<std::string, std::function<void()>> functions;
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::vector<Instruction> program;
    size_t pc = 0;
    TypeSystem typeSystem;

    void performUnaryOperation(const Instruction &instruction);
    void performBinaryOperation(const Instruction &instruction);
    void performComparisonOperation(const Instruction &instruction);
    void performLogicalOperation(const Instruction &instruction);
    void handleLoadConst(const ValuePtr &constantValue);
    void handleInterpolateString();
    void handlePrint();
    void handleHalt();
    void handleDeclareVariable(int32_t variableIndex);
    void handleLoadVariable(int32_t variableIndex);
    void handleStoreVariable(int32_t variableIndex);
    void handleDeclareFunction(const std::string &functionName);
    void handleCallFunction(const std::string &functionName);
    void handlePushArg(const Instruction &instruction);
    void handleJump();
    void handleJumpZero();
    void handleParallel(int32_t taskCount);
    void handleConcurrent(int32_t taskCount);
    void concurrent(std::vector<std::function<void()>> tasks);
};

#endif // STACK_BACKEND_HH
