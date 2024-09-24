#ifndef STACK_HH
#define STACK_HH

#include "../memory.hh"
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
    explicit StackBackend(std::vector<Instruction> &program);
    ~StackBackend();

    void run(const std::vector<Instruction> &program) override;
    void execute(const Instruction &instruction) override;
    void dumpRegisters() override;

    void setUnsafeMode(bool enable) { unsafeMode = enable; }
    bool isUnsafeMode() const { return unsafeMode; }

private:
    //    std::stack<ValuePtr> stack;
    //    std::vector<ValuePtr> constants;
    //    std::vector<ValuePtr> variables;
    std::stack<MemoryManager<>::Ref<Value>> stack;
    std::vector<MemoryManager<>::Ref<Value>> constants;
    std::vector<MemoryManager<>::Ref<Value>> variables;
    std::map<std::string, std::function<void()>> functions;
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::vector<Instruction> program;
    size_t pc = 0;
    TypeSystem typeSystem;
    bool unsafeMode = false;

    // Add MemoryManager
    MemoryManager<> memoryManager;
    MemoryManager<>::Region globalRegion;
    std::stack<MemoryManager<>::Region *> regionStack;

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

    void handleAlloc(const Instruction &instruction);
    void handleDealloc(const Instruction &instruction);
    void handleResize(const Instruction &instruction);

    // New methods for region management
    void pushRegion();
    void popRegion();
    MemoryManager<>::Region &currentRegion();

    //push ansd pop
    void push(const ValuePtr &valuePtr);

    ValuePtr pop();
    void clearStack();
};

#endif // STACK_BACKEND_HH
