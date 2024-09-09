#include "stack.hh"
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <type_traits>

void StackBackend::run(const std::vector<Instruction> &program)
{
    this->program = program;
    auto start_time = std::chrono::high_resolution_clock::now();
    try {
        pc = 0;
        auto start_time = std::chrono::high_resolution_clock::now();
        while (pc < program.size()) {
            const Instruction &instruction = program[pc];
            execute(instruction);
            pc++;
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
    std::stack<ValuePtr> tempStack = stack;
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
}

void StackBackend::performUnaryOperation(const Instruction &instruction)
{
    if (stack.empty()) {
        std::cerr << "Error: Invalid value stack for unary operation" << std::endl;
        return;
    }

    auto value = stack.top();
    stack.pop();

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

    stack.push(result);
}

void StackBackend::performBinaryOperation(const Instruction &instruction)
{
    if (stack.size() < 2) {
        std::cerr << "Error: Invalid value stack for binary operation" << std::endl;
        return;
    }

    auto value2 = stack.top();
    stack.pop();
    auto value1 = stack.top();
    stack.pop();

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

    stack.push(result);
}

void StackBackend::performLogicalOperation(const Instruction &instruction)
{
    if (stack.size() < 2) {
        std::cerr << "Error: Insufficient value stack for logical operation" << std::endl;
        return;
    }

    auto value2 = stack.top();
    stack.pop();
    auto value1 = stack.top();
    stack.pop();

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

    stack.push(result);
}

void StackBackend::performComparisonOperation(const Instruction &instruction)
{
    if (stack.size() < 2) {
        std::cerr << "Error: Insufficient value stack for comparison operation" << std::endl;
        return;
    }

    auto value2 = stack.top();
    stack.pop();
    auto value1 = stack.top();
    stack.pop();

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

    stack.push(result);
}

void StackBackend::handleLoadConst(const ValuePtr &constantValue)
{
    stack.push(constantValue);
}

void StackBackend::handleInterpolateString()
{
    if (stack.size() < 2) {
        std::cerr << "Error: Stack underflow during string interpolation" << std::endl;
        return;
    }

    // Pop the value to be interpolated
    auto value = stack.top();
    stack.pop();

    // Pop the template string
    auto templateStr = stack.top();
    stack.pop();

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
    stack.push(interpolatedString);
}

void StackBackend::handlePrint()
{
    if (stack.empty()) {
        std::cerr << "Error: value stack underflow" << std::endl;
        return;
    }
    std::visit([](const auto &value) { std::cout << "The result: " << value << std::endl; },
               stack.top()->data);
    stack.pop();
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
    auto value = stack.top();
    variables[variableIndex] = value;
    stack.pop();
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
            std::stack<ValuePtr> localStack;
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
    if (functions.find(functionName) == functions.end()) {
        std::cerr << "Error: Function not declared" << std::endl;
        return;
    }
    functions[functionName]();
}

void StackBackend::handlePushArg(const Instruction &instruction)
{
    stack.push(instruction.value);
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
    auto condition = stack.top();
    stack.pop();

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
    //int64_t offsetValue = std::get<int64_t>(offset->data);

    if (!conditionValue) {
        // Perform the jump
        // pc = offsetValue - 1; // Subtract 1 because pc will be incremented after this function
        if (std::holds_alternative<int64_t>(offset->data)) {
            int64_t offsetValue = std::get<int64_t>(offset->data);
            pc = offsetValue - 1;
        } else {
            std::cerr << "Error: After conversion, offset is still not Int64" << std::endl;
        }
    }
}

void StackBackend::concurrent(std::vector<std::function<void()>> tasks)
{
    // Start threads for each task
    for (auto &task : tasks) {
        threads.emplace_back(task);
    }

    // Join all threads
    for (auto &thread : threads) {
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

    // Create tasks for each part of the program
    for (int32_t i = 0; i < taskCount; ++i) {
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
