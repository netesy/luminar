#include "stack.hh"

void StackBackend::dumpRegisters() {
    std::cout << "Stack:\n";
    std::stack<Value> tempStack = stack;
    while (!tempStack.empty()) {
        auto value = tempStack.top();
        tempStack.pop();
        std::visit([](const auto &val) { std::cout << val; }, value);
        std::cout << "\n";
    }

    std::cout << "Constants:\n";
    for (size_t i = 0; i < constants.size(); ++i) {
        std::cout << "C-" << i << ": ";
        std::visit([](const auto &value) { std::cout << value; }, constants[i]);
        std::cout << "\n";
    }

    std::cout << "Variables:\n";
    for (size_t i = 0; i < variables.size(); ++i) {
        std::cout << "V-" << i << ": ";
        std::visit([](const auto &value) { std::cout << value; }, variables[i]);
        std::cout << "\n";
    }
}

void StackBackend::execute(const Instruction& instruction) {
    switch (instruction.opcode) {
        case NEGATE:
            performUnaryOperation(instruction);
            break;
        case ADD:
        case SUBTRACT:
        case MULTIPLY:
        case DIVIDE:
        case MODULUS:
            performBinaryOperation(instruction);
            break;
        case EQUAL:
        case NOT_EQUAL:
        case LESS_THAN:
        case LESS_THAN_OR_EQUAL:
        case GREATER_THAN:
        case GREATER_THAN_OR_EQUAL:
            performComparisonOperation(instruction);
            break;
        case AND:
        case OR:
        case NOT:
            performLogicalOperation(instruction);
            break;
        case LOAD_CONST:
            handleLoadConst(instruction.value);
            break;
        case PRINT:
            handlePrint();
            break;
        case HALT:
            handleHalt();
            return;
        case DECLARE_VARIABLE:
            handleDeclareVariable(instruction.value);
            break;
        case LOAD_VARIABLE:
            handleLoadVariable(instruction.value);
            break;
        case STORE_VARIABLE:
            handleStoreVariable(instruction.value);
            break;
        case WHILE_LOOP:
            handleWhileLoop();
            break;
        case PARALLEL:
            handleParallel(instruction.value);
            break;
        case CONCURRENT:
            handleConcurrent(instruction.value);
            break;
        default:
            std::cerr << "Unknown opcode." << std::endl;
    }
}

void StackBackend::performUnaryOperation(const Instruction& instruction) {
    if (stack.empty()) {
        std::cerr << "Error: Invalid value stack for unary operation" << std::endl;
        return;
    }

    auto value = stack.top();
    stack.pop();

    if (instruction.opcode == NEGATE) {
        if (std::holds_alternative<int32_t>(value)) {
            stack.push(-std::get<int32_t>(value));
        } else if (std::holds_alternative<double>(value)) {
            stack.push(-std::get<double>(value));
        } else {
            std::cerr << "Error: Unsupported type for NEGATE operation" << std::endl;
        }
    } else {
        std::cerr << "Error: Invalid unary operation opcode" << std::endl;
    }
}

void StackBackend::performBinaryOperation(const Instruction& instruction) {
    if (stack.size() < 2) {
        std::cerr << "Error: Invalid value stack for binary operation" << std::endl;
        return;
    }

    auto value2 = stack.top();
    stack.pop();
    auto value1 = stack.top();
    stack.pop();

    if (std::holds_alternative<int32_t>(value1) && std::holds_alternative<int32_t>(value2)) {
        int32_t reg1 = std::get<int32_t>(value1);
        int32_t reg2 = std::get<int32_t>(value2);

        switch (instruction.opcode) {
            case ADD:
                stack.push(reg1 + reg2);
                break;
            case SUBTRACT:
                stack.push(reg1 - reg2);
                break;
            case MULTIPLY:
                stack.push(reg1 * reg2);
                break;
            case DIVIDE:
                if (reg2 == 0) {
                    std::cerr << "Error: Division by zero" << std::endl;
                    return;
                }
                stack.push(reg1 / reg2);
                break;
            case MODULUS:
                if (reg2 == 0) {
                    std::cerr << "Error: Modulo by zero" << std::endl;
                    return;
                }
                stack.push(reg1 % reg2);
                break;
            default:
                std::cerr << "Error: Invalid binary operation opcode" << std::endl;
        }
    } else if (std::holds_alternative<double>(value1) && std::holds_alternative<double>(value2)) {
        double reg1 = std::get<double>(value1);
        double reg2 = std::get<double>(value2);

        switch (instruction.opcode) {
            case ADD:
                stack.push(reg1 + reg2);
                break;
            case SUBTRACT:
                stack.push(reg1 - reg2);
                break;
            case MULTIPLY:
                stack.push(reg1 * reg2);
                break;
            case DIVIDE:
                if (reg2 == 0.0) {
                    std::cerr << "Error: Division by zero" << std::endl;
                    return;
                }
                stack.push(reg1 / reg2);
                break;
            default:
                std::cerr << "Error: Invalid binary operation opcode" << std::endl;
        }
    } else {
        std::cerr << "Error: Unsupported types for binary operation" << std::endl;
    }
}

void StackBackend::performLogicalOperation(const Instruction& instruction) {
    if (stack.size() < 2) {
        std::cerr << "Error: Insufficient value stack for logical operation" << std::endl;
        return;
    }

    auto value2 = stack.top();
    stack.pop();
    auto value1 = stack.top();
    stack.pop();

    if (std::holds_alternative<bool>(value1) && std::holds_alternative<bool>(value2)) {
        bool reg1 = std::get<bool>(value1);
        bool reg2 = std::get<bool>(value2);

        switch (instruction.opcode) {
            case AND:
                stack.push(reg1 && reg2);
                break;
            case OR:
                stack.push(reg1 || reg2);
                break;
            default:
                std::cerr << "Error: Invalid logical operation opcode" << std::endl;
        }
    } else {
        std::cerr << "Error: Unsupported types for logical operation" << std::endl;
    }
}

void StackBackend::performComparisonOperation(const Instruction& instruction) {
    if (stack.size() < 2) {
        std::cerr << "Error: Insufficient value stack for comparison operation" << std::endl;
        return;
    }

    auto value2 = stack.top();
    stack.pop();
    auto value1 = stack.top();
    stack.pop();

    if (std::holds_alternative<int32_t>(value1) && std::holds_alternative<int32_t>(value2)) {
        int32_t reg1 = std::get<int32_t>(value1);
        int32_t reg2 = std::get<int32_t>(value2);

        switch (instruction.opcode) {
            case EQUAL:
                stack.push(reg1 == reg2);
                break;
            case NOT_EQUAL:
                stack.push(reg1 != reg2);
                break;
            case LESS_THAN:
                stack.push(reg1 < reg2);
                break;
            case LESS_THAN_OR_EQUAL:
                stack.push(reg1 <= reg2);
                break;
            case GREATER_THAN:
                stack.push(reg1 > reg2);
                break;
            case GREATER_THAN_OR_EQUAL:
                stack.push(reg1 >= reg2);
                break;
            default:
                std::cerr << "Error: Invalid comparison operation opcode" << std::endl;
        }
    } else if (std::holds_alternative<double>(value1) && std::holds_alternative<double>(value2)) {
        double reg1 = std::get<double>(value1);
        double reg2 = std::get<double>(value2);

        switch (instruction.opcode) {
            case EQUAL:
                stack.push(reg1 == reg2);
                break;
            case NOT_EQUAL:
                stack.push(reg1 != reg2);
                break;
            case LESS_THAN:
                stack.push(reg1 < reg2);
                break;
            case LESS_THAN_OR_EQUAL:
                stack.push(reg1 <= reg2);
                break;
            case GREATER_THAN:
                stack.push(reg1 > reg2);
                break;
            case GREATER_THAN_OR_EQUAL:
                stack.push(reg1 >= reg2);
                break;
            default:
                std::cerr << "Error: Invalid comparison operation opcode" << std::endl;
        }
    } else {
        std::cerr << "Error: Unsupported types for comparison operation" << std::endl;
    }
}

void StackBackend::handleLoadConst(unsigned int constantIndex) {
    if (constantIndex >= constants.size()) {
        std::cerr << "Error: Invalid constant index" << std::endl;
        return;
    }
    stack.push(constants[constantIndex]);
}

void StackBackend::handleDeclareVariable(unsigned int variableIndex) {
    if (variableIndex >= variables.size()) {
        variables.resize(variableIndex + 1);
    }
}

void StackBackend::handleLoadVariable(unsigned int variableIndex) {
    if (variableIndex >= variables.size()) {
        std::cerr << "Error: Invalid variable index" << std::endl;
        return;
    }
    stack.push(variables[variableIndex]);
}

void StackBackend::handleStoreVariable(unsigned int variableIndex) {
    if (variableIndex >= variables.size()) {
        std::cerr << "Error: Invalid variable index" << std::endl;
        return;
    }
    if (stack.empty()) {
        std::cerr << "Error: value stack underflow" << std::endl;
        return;
    }
    variables[variableIndex] = stack.top();
    stack.pop();
}

void StackBackend::handlePrint() {
    if (stack.empty()) {
        std::cerr << "Error: value stack underflow" << std::endl;
        return;
    }
    std::visit([](const auto &value) { std::cout << value << std::endl; }, stack.top());
    stack.pop();
}

void StackBackend::handleInput() {
    // Implementation for input
}

void StackBackend::handleOutput() {
    // Implementation for output
}


void StackBackend::concurrent(std::vector<std::function<void()>> tasks) {
    // Start threads for each task
    for (auto& task : tasks) {
        threads.emplace_back(task);
    }

// Join all threads
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    threads.clear();
}

void StackBackend::handleParallel(unsigned int taskCount) {
    std::vector<std::function<void()>> tasks;
    unsigned int instructionsPerTask = program.size() / taskCount;

    for (unsigned int i = 0; i < taskCount; ++i) {
        tasks.push_back([this, i, instructionsPerTask]() {
            unsigned int start = i * instructionsPerTask;
            unsigned int end = (i + 1) * instructionsPerTask;
            for (unsigned int j = start; j < end; ++j) {
                execute(program[j]);
            }
        });
    }

    concurrent(tasks);
}

void StackBackend::handleConcurrent(unsigned int taskCount) {
    std::vector<std::function<void()>> tasks;
    for (unsigned int i = 0; i < taskCount; ++i) {
        tasks.push_back([this, i]() {
            std::lock_guard<std::mutex> lock(this->mtx); // Lock for the duration of task
            this->pc = i;  // Assign each task its part of the program
          //  this->execute(program[taskCount]); // Run each part of the program concurrently
        });
    }
// Calculate the number of instructions per task
    unsigned int instructionsPerTask = program.size() / taskCount;

    // Create tasks for each part of the program
    for (unsigned int i = 0; i < taskCount; ++i) {
        tasks.push_back([this, i, instructionsPerTask]() {
            std::lock_guard<std::mutex> lock(this->mtx); // Lock for the duration of task
            unsigned int start = i * instructionsPerTask;
            unsigned int end = (i + 1) * instructionsPerTask;
            for (unsigned int j = start; j < end; ++j) {
                this->execute(program[j]);
            }
        });
    }
    concurrent(tasks);
}