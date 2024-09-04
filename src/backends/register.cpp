//#include "register.hh"

//void RegisterBackend::dumpRegisters() {
//    std::cout << "Registers:\n";
//    for (size_t i = 0; i < registers.size(); ++i) {
//        std::cout << "R-" << i << ": ";
//        std::visit([](const auto &value) { std::cout << value; }, registers[i]);
//        std::cout << "\n";
//    }

//    std::cout << "Constants:\n";
//    for (size_t i = 0; i < constants.size(); ++i) {
//        std::cout << "C-" << i << ": ";
//        std::visit([](const auto &value) { std::cout << value; }, constants[i]);
//        std::cout << "\n";
//    }

//    std::cout << "Variables:\n";
//    for (size_t i = 0; i < variables.size(); ++i) {
//        std::cout << "V-" << i << ": ";
//        std::visit([](const auto &value) { std::cout << value; }, variables[i]);
//        std::cout << "\n";
//    }
//}

//void RegisterBackend::execute(const Instruction& instruction) {
//    switch (instruction.opcode) {
//        case NEGATE:
//            performUnaryOperation(pc);
//            break;
//        case ADD:
//        case SUBTRACT:
//        case MULTIPLY:
//        case DIVIDE:
//        case MODULUS:
//            performBinaryOperation(pc);
//            break;
//        case EQUAL:
//        case NOT_EQUAL:
//        case LESS_THAN:
//        case LESS_THAN_OR_EQUAL:
//        case GREATER_THAN:
//        case GREATER_THAN_OR_EQUAL:
//            performComparisonOperation(pc);
//            break;
//        case AND:
//        case OR:
//        case NOT:
//            performLogicalOperation(pc);
//            break;
//        case LOAD_CONST:
//            handleLoadConst(pc);
//            break;
//        case PRINT:
//            handlePrint();
//            break;
//        case HALT:
//            handleHalt();
//            return;
//        case DECLARE_VARIABLE:
//            handleDeclareVariable(pc);
//            break;
//        case LOAD_VARIABLE:
//            handleLoadVariable(pc);
//            break;
//        case STORE_VARIABLE:
//            handleStoreVariable(pc);
//            break;
//        case WHILE_LOOP:
//            handleWhileLoop();
//            break;
//        case PARALLEL:
//            handleParallel(pc);
//            break;
//        case CONCURRENT:
//            handleConcurrent(pc);
//            break;
//        default:
//            std::cerr << "Unknown opcode." << std::endl;
//    }
//}

//void RegisterBackend::performUnaryOperation(int op) {
//    int opcode = program[op].opcode;
//    if (registers.empty()) {
//        std::cerr << "Error: Invalid operand register for unary operation" << std::endl;
//        return;
//    }

//    auto value = registers.back();
//    registers.pop_back();

//    if (opcode == NEGATE) {
//        if (std::holds_alternative<int32_t>(value)) {
//            registers.push_back(-std::get<int32_t>(value));
//        } else if (std::holds_alternative<double>(value)) {
//            registers.push_back(-std::get<double>(value));
//        } else {
//            std::cerr << "Error: Unsupported type for NEGATE operation" << std::endl;
//        }
//    } else {
//        std::cerr << "Error: Invalid unary operation opcode" << std::endl;
//    }
//}

//void RegisterBackend::performBinaryOperation(int op) {
//    int opcode = program[op].opcode;
//    if (registers.size() < 2) {
//        std::cerr << "Error: Invalid operand registers for binary operation" << std::endl;
//        return;
//    }

//    auto value2 = registers.back();
//    registers.pop_back();
//    auto value1 = registers.back();
//    registers.pop_back();

//    if (std::holds_alternative<int32_t>(value1) && std::holds_alternative<int32_t>(value2)) {
//        int32_t reg1 = std::get<int32_t>(value1);
//        int32_t reg2 = std::get<int32_t>(value2);

//        switch (opcode) {
//            case ADD:
//                registers.push_back(reg1 + reg2);
//                break;
//            case SUBTRACT:
//                registers.push_back(reg1 - reg2);
//                break;
//            case MULTIPLY:
//                registers.push_back(reg1 * reg2);
//                break;
//            case DIVIDE:
//                if (reg2 == 0) {
//                    std::cerr << "Error: Division by zero" << std::endl;
//                    return;
//                }
//                registers.push_back(reg1 / reg2);
//                break;
//            case MODULUS:
//                if (reg2 == 0) {
//                    std::cerr << "Error: Modulo by zero" << std::endl;
//                    return;
//                }
//                registers.push_back(reg1 % reg2);
//                break;
//            default:
//                std::cerr << "Error: Invalid binary operation opcode" << std::endl;
//        }
//    } else if (std::holds_alternative<double>(value1) && std::holds_alternative<double>(value2)) {
//        double reg1 = std::get<double>(value1);
//        double reg2 = std::get<double>(value2);

//        switch (opcode) {
//            case ADD:
//                registers.push_back(reg1 + reg2);
//                break;
//            case SUBTRACT:
//                registers.push_back(reg1 - reg2);
//                break;
//            case MULTIPLY:
//                registers.push_back(reg1 * reg2);
//                break;
//            case DIVIDE:
//                if (reg2 == 0.0) {
//                    std::cerr << "Error: Division by zero" << std::endl;
//                    return;
//                }
//                registers.push_back(reg1 / reg2);
//                break;
//            default:
//                std::cerr << "Error: Invalid binary operation opcode" << std::endl;
//        }
//    } else {
//        std::cerr << "Error: Unsupported types for binary operation" << std::endl;
//    }
//}

//void RegisterBackend::run(const std::vector<Instruction>& program)  {
//    this->program = program; // Store the program in the instance
//    try {
//        pc = 0; // Reset program counter
//        while (pc < program.size()) {
//            const Instruction& instruction = program[pc];
//            execute(instruction);
//            pc++;
//        }
//    } catch (const std::exception& ex) {
//        std::cerr << "Exception occurred during VM execution: " << ex.what() << std::endl;
//    }
//}

//void RegisterBackend::performLogicalOperation(int op) {
//    int opcode = program[op].opcode;
//    if (registers.size() < 2) {
//        std::cerr << "Error: Insufficient operand registers for logical operation" << std::endl;
//        return;
//    }

//    auto value2 = registers.back();
//    registers.pop_back();
//    auto value1 = registers.back();
//    registers.pop_back();

//    if (std::holds_alternative<bool>(value1) && std::holds_alternative<bool>(value2)) {
//        bool reg1 = std::get<bool>(value1);
//        bool reg2 = std::get<bool>(value2);

//        switch (opcode) {
//            case AND:
//                registers.push_back(reg1 && reg2);
//                break;
//            case OR:
//                registers.push_back(reg1 || reg2);
//                break;
//            default:
//                std::cerr << "Error: Invalid logical operation opcode" << std::endl;
//        }
//    } else {
//        std::cerr << "Error: Unsupported types for logical operation" << std::endl;
//    }
//}

//void RegisterBackend::performComparisonOperation(int op) {
//    int opcode = program[op].opcode;
//    if (registers.size() < 2) {
//        std::cerr << "Error: Insufficient operand registers for comparison operation" << std::endl;
//        return;
//    }

//    auto value2 = registers.back();
//    registers.pop_back();
//    auto value1 = registers.back();
//    registers.pop_back();

//    if (std::holds_alternative<int32_t>(value1) && std::holds_alternative<int32_t>(value2)) {
//        int32_t reg1 = std::get<int32_t>(value1);
//        int32_t reg2 = std::get<int32_t>(value2);

//        switch (opcode) {
//            case EQUAL:
//                registers.push_back(reg1 == reg2);
//                break;
//            case NOT_EQUAL:
//                registers.push_back(reg1 != reg2);
//                break;
//            case LESS_THAN:
//                registers.push_back(reg1 < reg2);
//                break;
//            case LESS_THAN_OR_EQUAL:
//                registers.push_back(reg1 <= reg2);
//                break;
//            case GREATER_THAN:
//                registers.push_back(reg1 > reg2);
//                break;
//            case GREATER_THAN_OR_EQUAL:
//                registers.push_back(reg1 >= reg2);
//                break;
//            default:
//                std::cerr << "Error: Invalid comparison operation opcode" << std::endl;
//        }
//    } else if (std::holds_alternative<double>(value1) && std::holds_alternative<double>(value2)) {
//        double reg1 = std::get<double>(value1);
//        double reg2 = std::get<double>(value2);

//        switch (opcode) {
//            case EQUAL:
//                registers.push_back(reg1 == reg2);
//                break;
//            case NOT_EQUAL:
//                registers.push_back(reg1 != reg2);
//                break;
//            case LESS_THAN:
//                registers.push_back(reg1 < reg2);
//                break;
//            case LESS_THAN_OR_EQUAL:
//                registers.push_back(reg1 <= reg2);
//                break;
//            case GREATER_THAN:
//                registers.push_back(reg1 > reg2);
//                break;
//            case GREATER_THAN_OR_EQUAL:
//                registers.push_back(reg1 >= reg2);
//                break;
//            default:
//                std::cerr << "Error: Invalid comparison operation opcode" << std::endl;
//        }
//    } else {
//        std::cerr << "Error: Unsupported types for comparison operation" << std::endl;
//    }
//}

//void RegisterBackend::handleLoadConst(unsigned int constantIndex) {
//    if (constantIndex >= constants.size()) {
//        std::cerr << "Error: Invalid constant index" << std::endl;
//        return;
//    }

//    registers.push_back(constants[constantIndex]);
//}

//void RegisterBackend::handleStoreVariable(unsigned int variableIndex) {
//    if (variableIndex >= variables.size()) {
//        std::cerr << "Error: Invalid variable index during storing" << std::endl;
//        return;
//    }

//    variables[variableIndex] = registers.back();
//    registers.pop_back();
//}

//void RegisterBackend::handlePrint() {
//    if (registers.empty()) {
//        std::cerr << "Error: No value to print" << std::endl;
//        return;
//    }

//    auto value = registers.back();
//    registers.pop_back();

//    std::visit([](auto &&arg) {
//        using T = std::decay_t<decltype(arg)>;
//        if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, double> || std::is_same_v<T, std::string>) {
//            std::cout << "PRINT: " << arg << std::endl;
//        } else if constexpr (std::is_same_v<T, bool>) {
//            std::cout << "PRINT: " << std::boolalpha << arg << std::endl;
//        } else {
//            std::cerr << "Unsupported type for PRINT operation" << std::endl;
//        }
//    }, value);
//}

//void RegisterBackend::handleDeclareVariable(unsigned int variableIndex) {
//    if (variableIndex >= variables.size()) {
//        variables.resize(variableIndex + 1);
//    }

//    variables[variableIndex] = {}; // Initialize with default value
//}

//void RegisterBackend::handleLoadVariable(unsigned int variableIndex) {
//    if (variableIndex >= variables.size()) {
//        std::cerr << "Error: Invalid variable index during loading" << std::endl;
//        return;
//    }

//    registers.push_back(variables[variableIndex]);
//}

//void RegisterBackend::handleWhileLoop() {
//    int loopStart = pc - 1;

//    while (true) {
//        auto conditionInstruction = program[pc];
//        execute(conditionInstruction);

//        auto condition = registers.back();
//        registers.pop_back();

//        if (std::holds_alternative<bool>(condition) && std::get<bool>(condition)) {
//            pc++;
//            auto bodyInstruction = program[pc];
//            execute(bodyInstruction);
//        } else {
//            break;
//        }

//        pc = loopStart;
//    }
//}

//void RegisterBackend::handleHalt() {
//    std::cout << "Halt operation" << std::endl;
//}

//void RegisterBackend::handleInput() {
//    // Implement input handling
//}

//void RegisterBackend::handleOutput() {
//    // Implement output handling
//}

//void RegisterBackend::concurrent(std::vector<std::function<void()>> tasks) {
//    for (auto& task : tasks) {
//        threads.emplace_back(task);
//    }

//    for (auto& thread : threads) {
//        if (thread.joinable()) {
//            thread.join();
//        }
//    }

//    threads.clear();
//}

//void RegisterBackend::handleParallel(unsigned int taskCount) {
//    std::vector<std::function<void()>> tasks;
//    unsigned int instructionsPerTask = program.size() / taskCount;

//    for (unsigned int i = 0; i < taskCount; ++i) {
//        tasks.push_back([this, i, instructionsPerTask]() {
//            unsigned int start = i * instructionsPerTask;
//            unsigned int end = (i + 1) * instructionsPerTask;
//            for (unsigned int j = start; j < end; ++j) {
//                execute(program[j]);
//            }
//        });
//    }

//    concurrent(tasks);
//}

//void RegisterBackend::handleConcurrent(unsigned int taskCount) {
//    std::vector<std::function<void()>> tasks;
//    // for (unsigned int i = 0; i < taskCount; ++i) {
//    //     tasks.push_back([this, i]() {
//    //         std::lock_guard<std::mutex> lock(this->mtx); // Lock for the duration of task
//    //         this->pc = i;  // Assign each task its part of the program
//    //         this->exe();  // Run each part of the program concurrently
//    //     });
//    // }
//    // Calculate the number of instructions per task
//    unsigned int instructionsPerTask = program.size() / taskCount;

//    // Create tasks for each part of the program
//    for (unsigned int i = 0; i < taskCount; ++i) {
//        tasks.push_back([this, i, instructionsPerTask]() {
//            std::lock_guard<std::mutex> lock(this->mtx); // Lock for the duration of task
//            unsigned int start = i * instructionsPerTask;
//            unsigned int end = (i + 1) * instructionsPerTask;
//            for (unsigned int j = start; j < end; ++j) {
//                this->execute(program[j]);
//            }
//        });
//    }

//    concurrent(tasks);
//}
