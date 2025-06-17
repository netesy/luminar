#include "stack.hh"
#include "../function.hh"
#include <chrono>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <variant>
#include "../memory.hh"

// Define the thread_local static member outside the class
//thread_local DefaultAllocator::ThreadCache DefaultAllocator::thread_cache;
//thread_local DefaultAllocator::ThreadCache DefaultAllocator::thread_cache;


StackBackend::~StackBackend()
{
    clearStack();
    while (!regionStack.empty()) {
        //  std::cout << "Popping region" << std::endl;
        popRegion();
    }
    memoryManager.analyzeMemoryUsage();
}

void StackBackend::run(const std::vector<Instruction> &program)
{
    this->program = program;
    auto start_time = std::chrono::high_resolution_clock::now();
    try {
        pc = 0;

        //program[pc].debug();
        auto start_time = std::chrono::high_resolution_clock::now();
        while (pc < this->program.size()) {
            const Instruction &instruction = this->program[pc];

            if (instruction.opcode == HALT) {
                std::cout << "Program halted normally." << std::endl;
                break;
            }


            execute(instruction);
            pc++;
          //  instruction.debug();
        }
        /// program[pc].debug();
        if (pc >= this->program.size()) {
            std::cerr << "Warning: Reached end of program without HALT instruction." << std::endl;
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "VM Execution completed in " << duration.count() << " microseconds."
                  << std::endl;
    } catch (const std::exception &ex) {
         handleExecutionError(ex);
           }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "VM ran for a total of  " << duration.count() << " microseconds." << std::endl;

    // Cleanup resources
    threads.clear();
    channels.clear();

    // Print memory statistics
    memoryManager.analyzeMemoryUsage();
    //memoryManager.printStatistics();
}

void StackBackend::execute(const Instruction &instruction)
{
    std::cout << "DEBUG: Executing instruction: " << instruction.opcodeToString(instruction.opcode) << std::endl;
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
    case LOAD_VALUE:
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
        break;
    case INVOKE_FUNCTION:
        handleCallFunction(std::get<std::string>(instruction.value->data));
        break;
    case RETURN:
        handleReturnFuction();
        break;
    case PUSH_ARGS:
        handlePushArg(instruction);
        break;
    case Opcode::CREATE_PARAM_FRAME: {
        std::string funcName = std::get<std::string>(instruction.value->data);
        function.pushParameterFrame(funcName, {});
        break;
    }

    case Opcode::STORE_PARAM:
        try {
            std::string paramName = std::get<std::string>(instruction.value->data);
            ValuePtr value = pop();

            std::string currentFunc = function.getCurrentFunctionName();

            auto funcInfo = function.getFunction(currentFunc);
            if (!funcInfo) {
                throw std::runtime_error("Function not found: " + currentFunc);
            }

            // Get current parameters
            auto currentParams = function.getCurrentParameters();
            currentParams[paramName] = value;

            std::vector<ValuePtr> orderedParams;
            for (const auto &param : funcInfo->parameters) {
                auto it = currentParams.find(param.name);
                if (it != currentParams.end()) {
                    orderedParams.push_back(it->second);
                } else {
                    std::cout << "Missing parameter: " << param.name << std::endl;
                }
            }

            // Replace parameter frame
            function.popParameterFrame();
            function.pushParameterFrame(currentFunc, orderedParams);

        } catch (const std::exception &e) {
            throw std::runtime_error("Error storing parameter: " + std::string(e.what()));
        }
        break;

    case Opcode::LOAD_PARAM: {
        std::string paramName = std::get<std::string>(instruction.value->data);
        ValuePtr value = function.getParameter(paramName);
        push(value);
        break;
    }

    case Opcode::POP_PARAM_FRAME: {
        function.popParameterFrame();
        break;
    }
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
    case Opcode::PARALLEL_CORES:
        configureParallelCores(instruction);
        break;

    case Opcode::CHANNEL_DEFINE:
        defineChannel(instruction);
        break;

    case Opcode::ERROR_STRATEGY:
        configureErrorStrategy(instruction);
       break;

    case Opcode::TASK_PARALLEL:
        executeParallelTask(instruction);
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

    // std::cout << "Functions:\n";
    // for (const auto &[name, _] : functions) {
    //     std::cout << "Function: " << name << "\n";
    // }
    std::cout << "End of Dump Registers\n";
}

void StackBackend::performUnaryOperation(const Instruction &instruction)
{
    if (stack.empty()) {
        std::cerr << "Error: Invalid value stack for unary operation" << std::endl;
        return;
    }

    auto value = pop();

    ValuePtr result = memoryManager.makeRef<Value>(*regionStack.top());
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

    std::cout << "[DEBUG] performBinaryOperation: Starting operation "
    << static_cast<int>(instruction.opcode) << std::endl;

if (stack.size() < 2) {
std::cerr << "[ERROR] Stack underflow in binary operation" << std::endl;
throw std::runtime_error("Stack underflow in binary operation");
}

// ValuePtr right = pop();
// ValuePtr left = pop();

// std::cout << "[DEBUG] performBinaryOperation: Left type: "
//     << (left && left->type ? left->type->toString() : "null")
//     << ", Right type: "
//     << (right && right->type ? right->type->toString() : "null")
//     << std::endl;

//     if (stack.size() < 2) {
//         std::cerr << "Error: Invalid value stack for binary operation" << std::endl;
//         return;
//     }

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

    try {
        if (commonType->tag == TypeTag::Int || commonType->tag == TypeTag::Int32 ||
            commonType->tag == TypeTag::Int64) {

            int64_t v1 = 0, v2 = 0;

            // Safely convert value1 to int64_t using the overloaded visitor
            std::visit(overloaded{
                [&](int8_t val) { v1 = val; },
                [&](int16_t val) { v1 = val; },
                [&](int32_t val) { v1 = val; },
                [&](int64_t val) { v1 = val; },
                [&](uint8_t val) { v1 = val; },
                [&](uint16_t val) { v1 = val; },
                [&](uint32_t val) { v1 = val; },
                [&](uint64_t val) { v1 = static_cast<int64_t>(val); },
                [](const auto&) {
                    throw std::runtime_error("Unsupported type for integer operation");
                }
            }, value1->data);

            // Safely convert value2 to int64_t using the overloaded visitor
            std::visit(overloaded{
                [&](int8_t val) { v2 = val; },
                [&](int16_t val) { v2 = val; },
                [&](int32_t val) { v2 = val; },
                [&](int64_t val) { v2 = val; },
                [&](uint8_t val) { v2 = val; },
                [&](uint16_t val) { v2 = val; },
                [&](uint32_t val) { v2 = val; },
                [&](uint64_t val) { v2 = static_cast<int64_t>(val); },
                [](const auto&) {
                    throw std::runtime_error("Unsupported type for integer operation");
                }
            }, value2->data);

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
        } else if (commonType->tag == TypeTag::Float64 || commonType->tag == TypeTag::Float32) {
            double v1 = 0.0, v2 = 0.0;

            // Safely convert value1 to double using the overloaded visitor
            std::visit(overloaded{
                [&](int8_t val) { v1 = static_cast<double>(val); },
                [&](int16_t val) { v1 = static_cast<double>(val); },
                [&](int32_t val) { v1 = static_cast<double>(val); },
                [&](int64_t val) { v1 = static_cast<double>(val); },
                [&](uint8_t val) { v1 = static_cast<double>(val); },
                [&](uint16_t val) { v1 = static_cast<double>(val); },
                [&](uint32_t val) { v1 = static_cast<double>(val); },
                [&](uint64_t val) { v1 = static_cast<double>(val); },
                [&](float val) { v1 = val; },
                [&](double val) { v1 = val; },
                [](const auto&) {
                    throw std::runtime_error("Unsupported type for float operation");
                }
            }, value1->data);

            // Safely convert value2 to double using the overloaded visitor
            std::visit(overloaded{
                [&](int8_t val) { v2 = static_cast<double>(val); },
                [&](int16_t val) { v2 = static_cast<double>(val); },
                [&](int32_t val) { v2 = static_cast<double>(val); },
                [&](int64_t val) { v2 = static_cast<double>(val); },
                [&](uint8_t val) { v2 = static_cast<double>(val); },
                [&](uint16_t val) { v2 = static_cast<double>(val); },
                [&](uint32_t val) { v2 = static_cast<double>(val); },
                [&](uint64_t val) { v2 = static_cast<double>(val); },
                [&](float val) { v2 = val; },
                [&](double val) { v2 = val; },
                [](const auto&) {
                    throw std::runtime_error("Unsupported type for float operation");
                }
            }, value2->data);

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

    } catch (const std::exception& e) {
        std::cerr << "Error in binary operation: " << e.what() << std::endl;
        // Push back the values to maintain stack consistency
        push(std::move(value1));
        push(std::move(value2));
    }
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

}

void StackBackend::handleLoadConst(const ValuePtr &constantValue)
{
    if (!constantValue) {
        std::cerr << "[ERROR] handleLoadConst: constantValue is null" << std::endl;
        return;
    }

    if (!constantValue->type) {
        std::cerr << "[ERROR] handleLoadConst: constantValue type is null" << std::endl;
        return;
    }

    try {
        std::cout << "[DEBUG] handleLoadConst: Creating value copy of type: " 
                  << constantValue->type->toString() << std::endl;

        // Create the value in the current region
        Value* rawValue = currentRegion().create<Value>(*constantValue);
        if (!rawValue) {
            std::cerr << "[ERROR] Failed to allocate value in region" << std::endl;
            return;
        }

        // Create a shared_ptr with a custom deleter that will call the destructor
        // but not deallocate memory (the region will handle that)
        ValuePtr valueCopy(rawValue, [](Value* ptr) {
            if (ptr) {
                ptr->~Value();
                // Memory will be freed when the region is destroyed
            }
        });

        std::cout << "[DEBUG] Successfully created value copy at " 
                  << rawValue << std::endl;

        // Push the copy onto the stack
        push(valueCopy);
        std::cout << "[DEBUG] Value pushed to stack" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[EXCEPTION] in handleLoadConst: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[UNKNOWN EXCEPTION] in handleLoadConst" << std::endl;
        throw;
    }
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
    std::cout << "[DEBUG] Creating interpolated string value" << std::endl;
    ValuePtr interpolatedString;
    try {
        // Create with type first
        interpolatedString = std::make_shared<Value>(typeSystem.STRING_TYPE);
        // Then assign the string data
        interpolatedString->data = result;
        std::cout << "[DEBUG] Successfully created interpolated string value" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to create interpolated string value: " << e.what() << std::endl;
        throw;
    }

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

void StackBackend::handleCallFunction(const std::string &functionName)
{
 if (!function.hasFunction(functionName)) {
        throw std::runtime_error("Function not found: " + functionName);
    }

    // Get function info
    auto functionInfo = function.getFunction(functionName);
    if (!functionInfo) {
        throw std::runtime_error("Function not found: " + functionName);
    }

    // Save current execution context
    // Store PC, program size, and current function's end point
    callStack.push({pc, program.size()});

    // Create new stack frame for function execution
    std::vector<ValuePtr> args;

    // Get parameters from the current parameter frame
    try {
        auto currentParams = function.getCurrentParameters();
        for (const auto &paramInfo : functionInfo->parameters) {
            auto paramValue = currentParams.find(paramInfo.name);
            if (paramValue != currentParams.end()) {
                args.push_back(paramValue->second);
            } else if (paramInfo.isOptional) {
                args.push_back(paramInfo.defaultValue);
            } else {
                throw std::runtime_error("Missing required parameter: " + paramInfo.name);
            }
        }
    } catch (const std::exception &e) {
        if (!functionInfo->parameters.empty()) {
            throw std::runtime_error("No parameters provided for function: " + functionName + " " + std::string(e.what()));
        }
    }

    if (functionInfo->isBuiltin) {
        // Execute built-in function
        ValuePtr result = function.executeBuiltin(functionName, args);
        if (result) {
            auto linearResult = memoryManager.makeLinear<Value>(currentRegion(), *result);
            auto sharedResult = std::make_shared<Value>(*linearResult);
            push(sharedResult);
        }
        // Restore context immediately for built-ins
        auto [savedPC, savedProgramSize] = callStack.top();
        callStack.pop();
        pc = savedPC;
    } else {
        // Get function body
        auto functionBody = function.getFunctionBody(functionName);
        if (!functionBody || functionBody->empty()) {
            throw std::runtime_error("Function body not found or empty for: " + functionName);
        }

        std::cout << "Executing function: " << functionName << "\n";
        // Create a temporary vector for the full program
        std::vector<Instruction> newProgram;

        // Reserve space for efficiency
        newProgram.reserve(program.size() + functionBody->size() + 1);

        // Copy instructions up to current point
            if (pc >= program.size()) {
        throw std::out_of_range("Program counter out of bounds");
    }
        newProgram.insert(newProgram.end(), program.begin(), program.begin() + pc + 1);
    newProgram.insert(newProgram.end(), functionBody->begin(), functionBody->end());
    newProgram.insert(newProgram.end(), program.begin() + pc + 1, program.end());


        // Add function body instructions
        // for (const auto& instr : *functionBody) {

        //    std::cout << "Adding instruction: ";
        //    instr.debug();
        //    newProgram.push_back(instr);
        // }

        // Ensure RETURN is present
        bool hasReturn = false;
        for (const auto& instr : *functionBody) {
            if (instr.opcode == RETURN) {
                hasReturn = true;
                break;
            }
        }

        // // Add remaining instructions from original program
        // newProgram.insert(newProgram.end(), program.begin() + pc + 1, program.end());

        // Replace program with new combined program
            this->program = std::move(newProgram);

            pc = pc + 1; // Position at first instruction of inserted function}
    }
}


void StackBackend::handleReturnFuction()
{
    if (callStack.empty()) {
        throw std::runtime_error("Return statement outside function");
    }

    // Get return value if any
    ValuePtr returnValue = nullptr;
    if (!stack.empty()) {
        returnValue = pop();
    }

    // Clean up parameter frame
    function.popParameterFrame();

    // Restore previous context
    auto [savedPC, savedStackSize] = callStack.top();
    callStack.pop();

    // Clean up dynamic instructions
    program.resize(savedPC); // Restore program to its original state

    // Restore stack to previous size
    while (stack.size() > savedStackSize) {
        stack.pop();
    }

    // Push return value if exists
    if (returnValue) {
        push(returnValue);
    }

    // Update PC
    pc = savedPC;

    std::cout << "Returned from function to PC: " << returnValue << std::endl;
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
    if (regionStack.empty() || !regionStack.top()) {
        std::cerr << "Error: regionStack is empty or top is null in push()" << std::endl;
        //return;
    }
    return *regionStack.top();
}

void StackBackend::push(const ValuePtr &valuePtr)
{
    std::cout << "DEBUG: Pushing value to stack: ";
    auto &region = currentRegion();
    if (!valuePtr) {
        std::cerr << "Error: valuePtr is null before makeRef" << std::endl;
        return;
    }
    if (regionStack.empty() || !regionStack.top()) {
        std::cerr << "Error: regionStack is empty or top is null before makeRef" << std::endl;
        return;
    }
    std::cout << "[DEBUG] valuePtr address: " << static_cast<const void*>(valuePtr.get()) << std::endl;
    auto refValue = memoryManager.makeRef<Value>(region, *valuePtr);

    stack.push(refValue); // Push the converted value onto the stack
    std::cout << "DEBUG: Pushing value to stack: ";
}

ValuePtr StackBackend::pop()
{
    // if (stack.empty()) {
    //     std::cerr << "Error: Stack underflow" << std::endl;
    //     return nullptr;
    // }

    // auto refValue = stack.top(); // Pop the value from the stack
    // stack.pop();

    // return std::make_shared<Value>(*refValue); // Convert MemoryManager<>::Ref<Value> to ValuePtr
    if (stack.empty()) {
        std::cerr << "Error: Stack underflow" << std::endl;
        return nullptr;
    }

    auto refValue = stack.top();
    stack.pop();

    auto result = std::make_shared<Value>(*refValue);

    std::cout << "DEBUG: Popped value from stack: ";
   // debugPrintValue(result);
    std::cout << "DEBUG: Stack size after pop: " << stack.size() << std::endl;

    return result;
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

void StackBackend::defineChannel(const Instruction &instruction)
{
    // // Extract channel name and type
    // auto channelInfo = std::get<std::pair<std::string, std::string>>(instruction.value->data);
    // std::string name = channelInfo.first;
    // std::string typeStr = channelInfo.second;

    // auto channel = std::make_unique<ChannelConfig>();

    // // Determine channel type
    // if (typeStr == "unbuffered")
    //     channel->type = ChannelConfig::UNBUFFERED;
    // else if (typeStr == "buffered")
    //     channel->type = ChannelConfig::BUFFERED;
    // else if (typeStr == "synchronized")
    //     channel->type = ChannelConfig::SYNCHRONIZED;
    // else
    //     throw std::runtime_error("Unknown channel type: " + typeStr);

    // // Store the channel
    // channels[name] = std::move(channel);
}

void StackBackend::configureErrorStrategy(const Instruction &instruction)
{
    // Extract error strategy configuration
    // auto strategyInfo = std::get<std::pair<std::string, std::string>>(instruction.value->data);
    // std::string key = strategyInfo.first;
    // std::string value = strategyInfo.second;

    // if (key == "mode") {
    //     if (value == "continue")
    //         currentErrorStrategy.action = ErrorStrategy::CONTINUE;
    //     else if (value == "stop")
    //         currentErrorStrategy.action = ErrorStrategy::STOP;
    //     else if (value == "retry")
    //         currentErrorStrategy.action = ErrorStrategy::RETRY;
    // }
}

void StackBackend::executeParallelTask(const Instruction &instruction)
{
    // Extract task range from instruction
    auto rangeList = std::get<ListValue>(instruction.value->data);
    size_t taskStart = std::get<int32_t>(rangeList.elements[0]->data);
    size_t taskEnd = std::get<int32_t>(rangeList.elements[1]->data);

    // Create parallel task
    std::vector<std::thread> parallelTasks;
    std::atomic<bool> executionFailed{false};

    // Divide task into chunks based on available cores
    size_t taskRange = taskEnd - taskStart;
    size_t chunkSize = std::max<size_t>(1, taskRange / maxCores);

    for (int i = 0; i < maxCores && !executionFailed; ++i) {
        size_t start = taskStart + (i * chunkSize);
        size_t end = std::min(taskStart + ((i + 1) * chunkSize), taskEnd);

        parallelTasks.emplace_back([this, start, end, &executionFailed]() {
            try {
                for (size_t j = start; j < end; ++j) {
                    if (executionFailed)
                        break;

                    const Instruction &taskInstruction = program[j];
                    execute(taskInstruction);
                }
            } catch (const std::exception &ex) {
                handleTaskError(ex, executionFailed);
            }
        });
    }

    // Wait for all tasks to complete
    for (auto &task : parallelTasks) {
        if (task.joinable()) {
            task.join();
        }
    }

    // Check if execution should be halted
    if (executionFailed && currentErrorStrategy.action == ErrorStrategy::STOP) {
        throw std::runtime_error("Parallel task execution failed");
    }
}

void StackBackend::handleTaskError(const std::exception &ex, std::atomic<bool> &executionFailed)
{
    std::cerr << "Parallel task error: " << ex.what() << std::endl;

    switch (currentErrorStrategy.action) {
    case ErrorStrategy::CONTINUE:
        // Log error and continue
        break;
    case ErrorStrategy::STOP:
        // Set flag to stop further execution
        executionFailed = true;
        break;
    case ErrorStrategy::RETRY:
        // Implement retry logic (placeholder)
        break;
    }
}

void StackBackend::handleExecutionError(const std::exception &ex)
{
    std::cerr << "Exception occurred during VM execution: " << ex.what() << std::endl;

    // Additional error handling based on strategy
    switch (currentErrorStrategy.action) {
    case ErrorStrategy::CONTINUE:
        // Log and continue (though this might not be meaningful after an exception)
        break;
    case ErrorStrategy::STOP:
        // Rethrow to halt execution
        throw;
    case ErrorStrategy::RETRY:
        // Implement VM-level retry mechanism
        break;
    }
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
