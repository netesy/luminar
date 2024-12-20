#ifndef STACK_HH
#define STACK_HH

#include "../function.hh"
#include "../memory.hh"
#include "../types.hh"
#include "backend.hh"
#include <functional>
#include <mutex>
#include <queue>
#include <stack>
#include <string>
#include <thread>
#include <vector>

class StackBackend : public Backend
{
public:
    explicit StackBackend(std::vector<Instruction> &program, Functions& funcs);
    ~StackBackend();

    void run(const std::vector<Instruction> &program) override;
    void execute(const Instruction &instruction) override;
    void dumpRegisters() override;

    void setUnsafeMode(bool enable) { unsafeMode = enable; }
    bool isUnsafeMode() const { return unsafeMode; }

private:
    std::stack<std::pair<size_t, size_t>> callStack; // Stores (PC, stackSize) pairs
    // std::shared_ptr<Functions> functions;
    Functions function;
    std::stack<MemoryManager<>::Ref<Value>> stack;
    std::vector<MemoryManager<>::Ref<Value>> constants;
    std::vector<MemoryManager<>::Ref<Value>> variables;
    // Functions functions;
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::vector<Instruction> program;
    size_t pc = 0;
    TypeSystem typeSystem;
    std::shared_ptr<TypeSystem> typeSystems = std::make_shared<TypeSystem>();
    bool unsafeMode = false;

    // New parallel execution support structures
    struct ChannelConfig {
        enum Type { UNBUFFERED, BUFFERED, SYNCHRONIZED } type;
        std::mutex mutex;
        std::condition_variable condition;
        std::queue<ValuePtr> buffer;
    };

    struct ErrorStrategy {
        enum Action {
            CONTINUE,   // Skip failed tasks
            STOP,       // Halt entire execution
            RETRY       // Attempt to retry tasks
        } action = STOP;
        int maxRetries = 3;
        std::chrono::seconds retryDelay{5};
    };

    // Parallel execution configuration
    int maxCores = std::thread::hardware_concurrency();
    ErrorStrategy currentErrorStrategy;
    std::unordered_map<std::string, std::unique_ptr<ChannelConfig>> channels;

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
    void handleCallFunction(const std::string &functionName);
    void handleReturnFuction();
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
    ValuePtr createRange(const ValuePtr &start, const ValuePtr &end, const ValuePtr &step);
    bool insideFunctionDefinition();
    void configureParallelCores(const Instruction& instruction) {
        // Extract and set maximum cores for parallel execution
        maxCores = std::get<int32_t>(instruction.value->data);
        maxCores = std::max(1, std::min(maxCores,
                                        static_cast<int>(std::thread::hardware_concurrency())));
    }

    void defineChannel(const Instruction& instruction);
    void configureErrorStrategy(const Instruction& instruction);
    void executeParallelTask(const Instruction& instruction);
    void handleTaskError(const std::exception& ex, std::atomic<bool>& executionFailed);
    void handleExecutionError(const std::exception& ex);
};

#endif // STACK_BACKEND_HH
