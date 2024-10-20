#include "stack.hh"
#include <chrono>
#include <cmath>
#include <iostream>
#include <type_traits>

StackBackend::StackBackend(std::vector<Instruction> &program)
    : program(program)
    , memoryManager(true)
    , globalRegion(memoryManager)
{
    regionStack.push(&globalRegion);
}

StackBackend::~StackBackend()
{
    std::cout << "Starting StackBackend destruction" << std::endl;
    clearStack();
    memoryManager.printStatistics();
    while (!regionStack.empty()) {
        //  std::cout << "Popping region" << std::endl;
        popRegion();
    }
    std::cout << "StackBackend destruction complete" << std::endl;
}

void StackBackend::run(const std::vector<Instruction> &program)
{
    this->program = program;
    auto start_time = std::chrono::high_resolution_clock::now();
    try {
        pc = 0;
        auto start_time = std::chrono::high_resolution_clock::now();
        while (pc < program.size()) {
            const Instruction &instruction = program[pc];

            if (instruction.opcode == HALT) {
                std::cout << "Program halted normally." << std::endl;
                break;
            }
            instruction.debug();
            execute(instruction);
            pc++;
        }
        if (pc >= program.size()) {
            std::cerr << "Warning: Reached end of program without HALT instruction." << std::endl;
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "VM Execution completed in " << duration.count() << " microseconds."
                  << std::endl;
    } catch (const std::exception &ex) {
        std::cerr << "Exception occurred during VM execution: " << ex.what() << std::endl;
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "VM ran for a total of  " << duration.count() << " microseconds." << std::endl;

    // Print memory statistics
    memoryManager.~MemoryManager();
}

void StackBackend::execute(const Instruction &instruction)
{
    switch (instruction.opcode) {
    case NEGATE:
    case NOT:
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
        performLogicalOperation(instruction);
        break;
    case LOAD_CONST:
    case LOAD_STR:
    case BOOLEAN:
        handleLoadConst(instruction.value);
        break;
    case INTERPOLATE_STRING:
        handleInterpolateString();
        break;
    case PRINT:
        handlePrint();
        break;
    case HALT:
        handleHalt();
        return;
    case DECLARE_VARIABLE:
        handleDeclareVariable(std::get<int32_t>(instruction.value->data));
        break;
    case LOAD_VARIABLE:
        handleLoadVariable(std::get<int32_t>(instruction.value->data));
        break;
    case STORE_VARIABLE:
        handleStoreVariable(std::get<int32_t>(instruction.value->data));
        break;
    case DEFINE_FUNCTION:
        handleDeclareFunction(std::get<std::string>(instruction.value->data));
        break;
    case INVOKE_FUNCTION:
        handleCallFunction(std::get<std::string>(instruction.value->data));
        break;
    case PUSH_ARGS:
        handlePushArg(instruction);
        break;
    case JUMP:
        handleJump();
        break;
    case JUMP_IF_FALSE:
        handleJumpZero();
        break;
    case Opcode::MAKE_RANGE: {
        // Assume top three items on the stack are: step, end, start
        ValuePtr step = pop();
        ValuePtr end = pop();
        ValuePtr start = pop();

        // Perform range loop initialization (push range object to the stack if needed)
        ValuePtr range = createRange(start, end, step);
        push(range); // Push the range for iteration
        break;
    }
    case PARALLEL:
        handleParallel(std::get<int32_t>(instruction.value->data));
        break;
    case CONCURRENT:
        handleConcurrent(std::get<int32_t>(instruction.value->data));
        break;
    default:
        std::cerr << "Unknown opcode.: " << instruction.opcodeToString(instruction.opcode)
                  << std::endl;
    }
}

void StackBackend::dumpRegisters()
{
    std::cout << "Stack:\n";
    std::stack<MemoryManager<>::Ref<Value>> tempStack = stack;
    while (!tempStack.empty()) {
        auto value = tempStack.top();
        tempStack.pop();
        std::visit([](const auto &val) { std::cout << val; }, value->data);
        std::cout << "\n";
    }

    std::cout << "Constants:\n";
    for (size_t i = 0; i < constants.size(); ++i) {
        std::cout << "C-" << i << ": ";
        std::visit([](const auto &value) { std::cout << value; }, constants[i]->data);
        std::cout << "\n";
    }

    std::cout << "Variables:\n";
    for (size_t i = 0; i < variables.size(); ++i) {
        std::cout << "V-" << i << ": ";
        std::visit([](const auto &value) { std::cout << value; }, variables[i]->data);
        std::cout << "\n";
    }

    std::cout << "Functions:\n";
    for (const auto &[name, _] : functions) {
        std::cout << "Function: " << name << "\n";
    }
    std::cout << "End of Dump Registers\n";
}

void StackBackend::performUnaryOperation(const Instruction &instruction)
{
    if (stack.empty()) {
        std::cerr << "Error: Invalid value stack for unary operation" << std::endl;
        return;
    }

    auto value = pop();

    ValuePtr result = std::make_shared<Value>();
    result->type = value->type;

    switch (instruction.opcode) {
    case NEGATE:
        if (typeSystem.isCompatible(typeSystem.INT_TYPE, value->type)) {
            result->data = -std::get<int64_t>(value->data);
        } else if (typeSystem.isCompatible(typeSystem.FLOAT64_TYPE, value->type)) {
            result->data = -std::get<double>(value->data);
        } else {
            std::cerr << "Error: Unsupported type for NEGATE operation" << std::endl;
            return;
        }
        break;

    case NOT:
        if (typeSystem.isCompatible(typeSystem.BOOL_TYPE, value->type)) {
            result->data = !std::get<bool>(value->data);
        } else {
            std::cerr << "Error: Unsupported type for NOT operation" << std::endl;
            return;
        }
        break;

    default:
        std::cerr << "Error: Invalid unary operation opcode" << std::endl;
        return;
    }

    push(result);
}

void StackBackend::performBinaryOperation(const Instruction &instruction)
{
    if (stack.size() < 2) {
        std::cerr << "Error: Invalid value stack for binary operation" << std::endl;
        return;
    }

    auto value2 = pop();
    auto value1 = pop();

    // Get common type between the two values
    TypePtr commonType = typeSystem.getCommonType(value1->type, value2->type);
    if (!commonType) {
        std::cerr << "Error: Incompatible types for binary operation" << std::endl;
        return;
    }

    ValuePtr result = std::make_shared<Value>();
    result->type = commonType;

    if (commonType->tag == TypeTag::Int) {
        int64_t v1 = std::get<int64_t>(value1->data);
        int64_t v2 = std::get<int64_t>(value2->data);

        switch (instruction.opcode) {
        case ADD:
            result->data = v1 + v2;
            break;
        case SUBTRACT:
            result->data = v1 - v2;
            break;
        case MULTIPLY:
            result->data = v1 * v2;
            break;
        case DIVIDE:
            if (v2 == 0) {
                std::cerr << "Error: Division by zero" << std::endl;
                return;
            }
            result->data = v1 / v2;
            break;
        case MODULUS:
            if (v2 == 0) {
                std::cerr << "Error: Modulo by zero" << std::endl;
                return;
            }
            result->data = v1 % v2;
            break;
        default:
            std::cerr << "Error: Invalid binary operation opcode" << std::endl;
            return;
        }
    } else if (commonType->tag == TypeTag::Float64) {
        double v1 = std::get<double>(value1->data);
        double v2 = std::get<double>(value2->data);

        switch (instruction.opcode) {
        case ADD:
            result->data = v1 + v2;
            break;
        case SUBTRACT:
            result->data = v1 - v2;
            break;
        case MULTIPLY:
            result->data = v1 * v2;
            break;
        case DIVIDE:
            if (v2 == 0.0) {
                std::cerr << "Error: Division by zero" << std::endl;
                return;
            }
            result->data = v1 / v2;
            break;
        case MODULUS:
            result->data = std::fmod(v1, v2);
            break;
        default:
            std::cerr << "Error: Invalid binary operation opcode" << std::endl;
            return;
        }
    } else {
        std::cerr << "Error: Unsupported types for binary operation" << std::endl;
        return;
    }

    push(result);
}

void StackBackend::performLogicalOperation(const Instruction &instruction)
{
    if (stack.size() < 2) {
        std::cerr << "Error: Insufficient value stack for logical operation" << std::endl;
        return;
    }

    auto value2 = pop();

    auto value1 = pop();

    // Ensure both values are of type bool
    if (!typeSystem.isCompatible(typeSystem.BOOL_TYPE, value1->type)
        || !typeSystem.isCompatible(typeSystem.BOOL_TYPE, value2->type)) {
        std::cerr << "Error: Unsupported types for logical operation" << std::endl;
        return;
    }

    ValuePtr result = std::make_shared<Value>();
    result->type = typeSystem.BOOL_TYPE;

    bool v1 = std::get<bool>(value1->data);
    bool v2 = std::get<bool>(value2->data);

    switch (instruction.opcode) {
    case AND:
        result->data = v1 && v2;
        break;
    case OR:
        result->data = v1 || v2;
        break;
    default:
        std::cerr << "Error: Invalid logical operation opcode" << std::endl;
        return;
    }

    push(result);
}

void StackBackend::performComparisonOperation(const Instruction &instruction)
{
    if (stack.size() < 2) {
        std::cerr << "Error: Insufficient value stack for comparison operation" << std::endl;
        return;
    }

    auto value2 = pop();

    auto value1 = pop();

    // Get common type between the two values
    TypePtr commonType = typeSystem.getCommonType(value1->type, value2->type);
    if (!commonType) {
        std::cerr << "Error: Cannot compare values of different types" << std::endl;
        return;
    }

    ValuePtr result = std::make_shared<Value>();
    result->type = typeSystem.BOOL_TYPE;

    auto compareValues = [&](auto v1, auto v2) {
        switch (instruction.opcode) {
        case EQUAL:
            result->data = (v1 == v2);
            break;
        case NOT_EQUAL:
            result->data = (v1 != v2);
            break;
        case LESS_THAN:
            result->data = (v1 < v2);
            break;
        case LESS_THAN_OR_EQUAL:
            result->data = (v1 <= v2);
            break;
        case GREATER_THAN:
            result->data = (v1 > v2);
            break;
        case GREATER_THAN_OR_EQUAL:
            result->data = (v1 >= v2);
            break;
        default:
            std::cerr << "Error: Invalid comparison operation opcode" << std::endl;
            return false;
        }
        return true;
    };

    if (commonType->tag == TypeTag::Int) {
        if (!compareValues(std::get<int64_t>(value1->data), std::get<int64_t>(value2->data))) {
            return;
        }
    } else if (commonType->tag == TypeTag::Float64) {
        if (!compareValues(std::get<double>(value1->data), std::get<double>(value2->data))) {
            return;
        }
    } else if (commonType->tag == TypeTag::String) {
        if (!compareValues(std::get<std::string>(value1->data),
                           std::get<std::string>(value2->data))) {
            return;
        }
    } else {
        std::cerr << "Error: Unsupported type for comparison operation" << std::endl;
        return;
    }

    push(result);
}

void StackBackend::handleLoadConst(const ValuePtr &constantValue)
{
    //push(constantValue);
    auto linearValue = memoryManager.makeLinear<Value>(currentRegion(), *constantValue);
    auto sharedValue = std::make_shared<Value>(*linearValue);

    // Push the linear value onto the stack
    push(sharedValue);
}

void StackBackend::handleInterpolateString()
{
    if (stack.size() < 2) {
        std::cerr << "Error: Stack underflow during string interpolation" << std::endl;
        return;
    }

    // Pop the value to be interpolated
    auto value = pop();

    // Pop the template string
    auto templateStr = pop();

    std::string result;
    std::string templateString;

    // Convert template to string
    std::visit(
        [&templateString](const auto &v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>) {
                templateString = v;
            } else {
                std::cerr << "Error: Template is not a string" << std::endl;
            }
        },
        templateStr->data);

    // Find the first occurrence of {}
    size_t pos = templateString.find("{}");
    if (pos == std::string::npos) {
        std::cerr << "Error: No {} found in template string" << std::endl;
        return;
    }

    // Convert value to string
    std::string interpolatedValue = std::visit(
        [](const auto &v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return "null";
            } else if constexpr (std::is_same_v<T, bool>) {
                return v ? "true" : "false";
            } else if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(v);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return v;
            } else {
                return "Unsupported type";
            }
        },
        value->data);

    // Perform the interpolation
    result = templateString.substr(0, pos) + interpolatedValue + templateString.substr(pos + 2);

    // Create a new Value for the interpolated string
    ValuePtr interpolatedString = std::make_shared<Value>();
    interpolatedString->type = typeSystem.STRING_TYPE;
    interpolatedString->data = result;

    // Push the result back onto the stack
    push(interpolatedString);
}

void StackBackend::handlePrint()
{
    if (stack.empty()) {
        std::cerr << "Error: value stack underflow" << std::endl;
        return;
    }

    auto value = pop();

    auto printValue = [](const ValuePtr &val) {
        std::visit(
            [](const auto &v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double>) {
                    std::cout << v;
                } else if constexpr (std::is_same_v<T, std::string>) {
                    std::cout << "\"" << v << "\"";
                } else if constexpr (std::is_same_v<T, bool>) {
                    std::cout << (v ? "true" : "false");
                } else if constexpr (std::is_same_v<T, ListValue>) {
                    // This case should not be reached in this context
                    std::cout << "[nested list]";
                } else {
                    std::cout << "Unknown type";
                }
            },
            val->data);
    };

    if (value->type->tag == TypeTag::List) {
        const auto &list = std::get<ListValue>(value->data);
        std::cout << "[";
        for (size_t i = 0; i < list.elements.size(); ++i) {
            printValue(list.elements[i]);
            if (i < list.elements.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]";
    } else {
        printValue(value);
    }
    std::cout << std::endl;
}

void StackBackend::handleHalt()
{
    std::cout << "Execution halted." << std::endl;
    exit(0);
}

void StackBackend::handleDeclareVariable(int32_t variableIndex)
{
    if (variableIndex >= static_cast<int32_t>(variables.size())) {
        variables.resize(variableIndex + 1);
    }

    // Initialize the variable in the current region
    if (!variables[variableIndex].get()) {
        auto linearValue = memoryManager.makeLinear<Value>(currentRegion());
        variables[variableIndex] = memoryManager.makeRef<Value>(currentRegion());
    }
}

void StackBackend::handleLoadVariable(int32_t variableIndex)
{
    if (variableIndex >= static_cast<int32_t>(variables.size())) {
        std::cerr << "Error: Invalid variable index" << std::endl;
        return;
    }
    stack.push(variables[variableIndex]);
}

void StackBackend::handleStoreVariable(int32_t variableIndex)
{
    if (variableIndex >= static_cast<int32_t>(variables.size())) {
        variables.resize(variableIndex + 1);
    }
    if (stack.empty()) {
        std::cerr << "Error: value stack underflow" << std::endl;
        return;
    }
    //    auto value = pop();
    //    variables[variableIndex] = memoryManager.makeRef<Value>(currentRegion(), value);
    // Assuming pop returns ValuePtr
    ValuePtr valuePtr = pop();

    if (valuePtr) {
        // Extract the Value from ValuePtr
        const Value &value = *valuePtr;

        // Use the extracted Value to create a Ref<Value>
        variables[variableIndex] = memoryManager.makeRef<Value>(currentRegion(), value);
    } else {
        std::cerr << "Error: Null value pointer" << std::endl;
    }
}

void StackBackend::handleDeclareFunction(const std::string &functionName)
{
    if (functions.find(functionName) != functions.end()) {
        std::cerr << "Error: Function " << functionName << " already declared" << std::endl;
        return;
    }
    functions[functionName] = [this, functionName]() {
        auto it = std::find_if(program.begin(),
                               program.end(),
                               [functionName](const Instruction &instr) {
                                   return instr.opcode == Opcode::DEFINE_FUNCTION
                                          && std::get<std::string>(instr.value->data)
                                                 == functionName;
                               });

        if (it != program.end()) {
            size_t index = std::distance(program.begin(), it);
            std::stack<MemoryManager<>::Ref<Value>> localStack;
            std::swap(stack, localStack); // Save current stack state
            for (size_t i = index + 1; i < program.size() && program[i].opcode != Opcode::HALT;
                 ++i) {
                execute(program[i]);
            }
            std::swap(stack, localStack); // Restore previous stack state
        } else {
            std::cerr << "Error: Function not found" << std::endl;
        }
    };
}

void StackBackend::handleCallFunction(const std::string &functionName)
{
    pushRegion(); // Create a new region for the function call
    if (functions.find(functionName) == functions.end()) {
        std::cerr << "Error: Function not declared" << std::endl;
        popRegion();
        return;
    }
    functions[functionName]();
    popRegion();
}

void StackBackend::handlePushArg(const Instruction &instruction)
{
    push(instruction.value);
}

void StackBackend::handleJump()
{
    auto offset = program[this->pc].value;

    // Ensure offset is of type Int64 and convert if necessary
    if (!typeSystem.checkType(offset, typeSystem.INT64_TYPE)) {
        if (typeSystem.isCompatible(offset->type, typeSystem.INT64_TYPE)) {
            offset = typeSystem.convert(offset, typeSystem.INT64_TYPE);
            if (!typeSystem.checkType(offset, typeSystem.INT64_TYPE)) {
                std::cerr << "Error: Conversion to Int64 failed" << std::endl;
                return;
            }
        } else {
            std::cerr << "Error: Invalid jump offset type, expected Int64" << std::endl;
            return;
        }
    }

    // Perform the jump
    if (std::holds_alternative<int64_t>(offset->data)) {
        auto offsetValue = std::get<int64_t>(offset->data);
        pc += offsetValue;
    } else {
        std::cerr << "Error: After conversion, offset is still not Int64" << std::endl;
    }
}

void StackBackend::handleJumpZero()
{
    auto offset = program[this->pc].value;
    auto condition = pop();

    // Ensure condition is boolean
    if (!typeSystem.checkType(condition, typeSystem.BOOL_TYPE)) {
        if (typeSystem.isCompatible(condition->type, typeSystem.BOOL_TYPE)) {
            condition = typeSystem.convert(condition, typeSystem.BOOL_TYPE);
        } else {
            std::cerr << "Error: JUMP_IF_FALSE requires a boolean condition" << std::endl;
            return;
        }
    }

    // Ensure offset is of type Int64 and convert if necessary
    if (!typeSystem.checkType(offset, typeSystem.INT64_TYPE)) {
        if (typeSystem.isCompatible(offset->type, typeSystem.INT64_TYPE)) {
            offset = typeSystem.convert(offset, typeSystem.INT64_TYPE);
        } else {
            std::cerr << "Error: Invalid jump zero offset type, expected Int64" << std::endl;
            return;
        }
    }

    bool conditionValue = std::get<bool>(condition->data);

    if (!conditionValue) {
        // Perform the jump
        if (std::holds_alternative<int64_t>(offset->data)) {
            int64_t offsetValue = std::get<int64_t>(offset->data);
            pc = offsetValue - 1; // Subtract 1 because pc will be incremented after this function
        } else {
            std::cerr << "Error: After conversion, offset is still not Int64" << std::endl;
        }
    }
}

void StackBackend::pushRegion()
{
    regionStack.push(new MemoryManager<>::Region(memoryManager));
}

void StackBackend::popRegion()
{
    if (regionStack.size() > 1) { // Always keep the global region
        try {
            delete regionStack.top();
            regionStack.pop();
        } catch (const std::exception &e) {
            std::cerr << "Error during region cleanup: " << e.what() << std::endl;
        }
    }
}

MemoryManager<>::Region &StackBackend::currentRegion()
{
    return *regionStack.top();
}

void StackBackend::push(const ValuePtr &valuePtr)
{
    auto refValue = memoryManager.makeRef<Value>(currentRegion(), *valuePtr);

    stack.push(refValue); // Push the converted value onto the stack
}

ValuePtr StackBackend::pop()
{
    if (stack.empty()) {
        std::cerr << "Error: Stack underflow" << std::endl;
        return nullptr;
    }

    auto refValue = stack.top(); // Pop the value from the stack
    stack.pop();

    return std::make_shared<Value>(*refValue); // Convert MemoryManager<>::Ref<Value> to ValuePtr
}

void StackBackend::clearStack()
{
    std::cout << "Clearing stack" << std::endl;
    while (!stack.empty()) {
        try {
            stack.pop();
        } catch (const std::exception &e) {
            std::cerr << "Error clearing stack: " << e.what() << std::endl;
        }
    }
}

ValuePtr StackBackend::createRange(const ValuePtr &start, const ValuePtr &end, const ValuePtr &step)
{
    auto range = std::make_shared<Value>();
    range->type = std::make_shared<Type>(TypeTag::List); // Treat range as a list

    ListValue rangeList;
    int64_t begin = std::get<int64_t>(start->data);
    int64_t finish = std::get<int64_t>(end->data);
    int64_t stepValue = std::get<int64_t>(step->data);

    if (stepValue > 0) {
        for (int64_t i = begin; i <= finish; i += stepValue) {
            rangeList.elements.push_back(
                std::make_shared<Value>(Value{std::make_shared<Type>(TypeTag::Int), i}));
        }
    } else {
        for (int64_t i = begin; i >= finish; i += stepValue) {
            rangeList.elements.push_back(
                std::make_shared<Value>(Value{std::make_shared<Type>(TypeTag::Int), i}));
        }
    }

    range->data = rangeList;
    return range;
}

void StackBackend::concurrent(std::vector<std::function<void()>> tasks)
{
    for (auto &task : tasks) { // Start threads for each task
        threads.emplace_back(task);
    }

    for (auto &thread : threads) { // Join all threads
        if (thread.joinable()) {
            thread.join();
        }
    }

    threads.clear();
}
void StackBackend::handleParallel(int32_t taskCount)
{
    std::vector<std::function<void()>> tasks;
    unsigned int instructionsPerTask = program.size() / taskCount;

    for (int32_t i = 0; i < taskCount; ++i) {
        tasks.push_back([this, i, instructionsPerTask, &taskCount]() {
            unsigned int start = i * instructionsPerTask;
            unsigned int end = (i == taskCount - 1) ? program.size() : start + instructionsPerTask;

            for (unsigned int j = start; j < end; ++j) {
                const Instruction &instruction = program[j];
                execute(instruction);
            }
        });
    }

    concurrent(tasks);
}

void StackBackend::handleConcurrent(int32_t taskCount)
{
    std::vector<std::function<void()>> tasks;
    // Calculate the number of instructions per task
    unsigned int instructionsPerTask = program.size() / taskCount;

    for (int32_t i = 0; i < taskCount; ++i) { // Create tasks for each part of the program
        tasks.push_back([this, i, instructionsPerTask]() {
            int32_t start = i * instructionsPerTask;
            int32_t end = (i + 1) * instructionsPerTask;
            for (int32_t j = start; j < end; ++j) {
                std::lock_guard<std::mutex> lock(this->mtx); // Lock for the duration of task
                execute(program[j]);
            }
        });
    }

    concurrent(tasks);
}
