#include "vm.hh"
#include "helper.hh"
#include <iostream>
#include <variant>

void RegisterVM::dumpRegisters() {
    std::cout << "Registers:\n";
    for (size_t i = 0; i < registers.size(); ++i) {
        std::cout << "R-" << i << ": ";
        std::visit([](const auto &value) { std::cout << value; }, registers[i]);
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

void RegisterVM::run() {
    try {
        while (pc < program.size()) {
            const Instruction& instruction = program[pc];
            execute(instruction);
            pc++;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception occurred during VM execution: " << ex.what() << std::endl;
    }
}

void RegisterVM::execute(const Instruction& instruction) {
    switch (instruction.opcode) {
        case NEGATE:
            PerformUnaryOperation(pc);
            break;
        case ADD:
        case SUBTRACT:
        case MULTIPLY:
        case DIVIDE:
        case MODULUS:
            PerformBinaryOperation(pc);
            break;
        case EQUAL:
        case NOT_EQUAL:
        case LESS_THAN:
        case LESS_THAN_OR_EQUAL:
        case GREATER_THAN:
        case GREATER_THAN_OR_EQUAL:
            PerformComparisonOperation(pc);
            break;
        case AND:
        case OR:
        case NOT:
            PerformLogicalOperation(pc);
            break;
        case LOAD_CONST:
            HandleLoadConst(pc);
            break;
        case PRINT:
            print();
            break;
        case HALT:
            HandleHalt();
            return;
        case DECLARE_VARIABLE:
            HandleDeclareVariable(pc);
            break;
        case LOAD_VARIABLE:
            HandleLoadVariable(pc);
            break;
        case STORE_VARIABLE:
            HandleStoreVariable(pc);
            break;
        default:
            std::cerr << "Unknown opcode." << std::endl;
    }
}

void RegisterVM::PerformUnaryOperation(int op) {
    int opcode = program[op].opcode;
    if (registers.empty()) {
        std::cerr << "Error: Invalid operand register for unary operation" << std::endl;
        return;
    }

    auto value = registers.back();
    registers.pop_back();

    if (opcode == NEGATE) {
        if (std::holds_alternative<int32_t>(value)) {
            registers.push_back(-std::get<int32_t>(value));
        } else if (std::holds_alternative<double>(value)) {
            registers.push_back(-std::get<double>(value));
        } else {
            std::cerr << "Error: Unsupported type for NEGATE operation" << std::endl;
        }
    } else {
        std::cerr << "Error: Invalid unary operation opcode" << std::endl;
    }
}

void RegisterVM::PerformBinaryOperation(int op) {
    int opcode = program[op].opcode;
    if (registers.size() < 2) {
        std::cerr << "Error: Invalid operand registers for binary operation" << std::endl;
        return;
    }

    auto value2 = registers.back();
    registers.pop_back();
    auto value1 = registers.back();
    registers.pop_back();

    if (std::holds_alternative<int32_t>(value1) && std::holds_alternative<int32_t>(value2)) {
        int32_t reg1 = std::get<int32_t>(value1);
        int32_t reg2 = std::get<int32_t>(value2);

        switch (opcode) {
            case ADD:
                registers.push_back(reg1 + reg2);
                break;
            case SUBTRACT:
                registers.push_back(reg1 - reg2);
                break;
            case MULTIPLY:
                registers.push_back(reg1 * reg2);
                break;
            case DIVIDE:
                if (reg2 == 0) {
                    std::cerr << "Error: Division by zero" << std::endl;
                    return;
                }
                registers.push_back(reg1 / reg2);
                break;
            case MODULUS:
                if (reg2 == 0) {
                    std::cerr << "Error: Modulo by zero" << std::endl;
                    return;
                }
                registers.push_back(reg1 % reg2);
                break;
            default:
                std::cerr << "Error: Invalid binary operation opcode" << std::endl;
        }
    } else if (std::holds_alternative<double>(value1) && std::holds_alternative<double>(value2)) {
        double reg1 = std::get<double>(value1);
        double reg2 = std::get<double>(value2);

        switch (opcode) {
            case ADD:
                registers.push_back(reg1 + reg2);
                break;
            case SUBTRACT:
                registers.push_back(reg1 - reg2);
                break;
            case MULTIPLY:
                registers.push_back(reg1 * reg2);
                break;
            case DIVIDE:
                if (reg2 == 0.0) {
                    std::cerr << "Error: Division by zero" << std::endl;
                    return;
                }
                registers.push_back(reg1 / reg2);
                break;
            default:
                std::cerr << "Error: Invalid binary operation opcode" << std::endl;
        }
    } else {
        std::cerr << "Error: Unsupported types for binary operation" << std::endl;
    }
}

void RegisterVM::PerformLogicalOperation(int op) {
    int opcode = program[op].opcode;
    if (registers.size() < 2) {
        std::cerr << "Error: Insufficient operand registers for logical operation" << std::endl;
        return;
    }

    auto value2 = registers.back();
    registers.pop_back();
    auto value1 = registers.back();
    registers.pop_back();

    if (std::holds_alternative<bool>(value1) && std::holds_alternative<bool>(value2)) {
        bool reg1 = std::get<bool>(value1);
        bool reg2 = std::get<bool>(value2);

        switch (opcode) {
            case AND:
                registers.push_back(reg1 && reg2);
                break;
            case OR:
                registers.push_back(reg1 || reg2);
                break;
            default:
                std::cerr << "Error: Invalid logical operation opcode" << std::endl;
        }
    } else {
        std::cerr << "Error: Unsupported types for logical operation" << std::endl;
    }
}

void RegisterVM::PerformComparisonOperation(int op) {
    int opcode = program[op].opcode;
    if (registers.size() < 2) {
        std::cerr << "Error: Insufficient operand registers for comparison operation" << std::endl;
        return;
    }

    auto value2 = registers.back();
    registers.pop_back();
    auto value1 = registers.back();
    registers.pop_back();

    if (std::holds_alternative<int32_t>(value1) && std::holds_alternative<int32_t>(value2)) {
        int32_t reg1 = std::get<int32_t>(value1);
        int32_t reg2 = std::get<int32_t>(value2);

        switch (opcode) {
            case GREATER_THAN:
                registers.push_back(reg1 > reg2);
                break;
            case LESS_THAN:
                registers.push_back(reg1 < reg2);
                break;
            case GREATER_THAN_OR_EQUAL:
                registers.push_back(reg1 >= reg2);
                break;
            case LESS_THAN_OR_EQUAL:
                registers.push_back(reg1 <= reg2);
                break;
            case EQUAL:
                registers.push_back(reg1 == reg2);
                break;
            case NOT_EQUAL:
                registers.push_back(reg1 != reg2);
                break;
            default:
                std::cerr << "Error: Invalid comparison operation opcode" << std::endl;
        }
    } else if (std::holds_alternative<double>(value1) && std::holds_alternative<double>(value2)) {
        double reg1 = std::get<double>(value1);
        double reg2 = std::get<double>(value2);

        switch (opcode) {
            case GREATER_THAN:
                registers.push_back(reg1 > reg2);
                break;
            case LESS_THAN:
                registers.push_back(reg1 < reg2);
                break;
            case GREATER_THAN_OR_EQUAL:
                registers.push_back(reg1 >= reg2);
                break;
            case LESS_THAN_OR_EQUAL:
                registers.push_back(reg1 <= reg2);
                break;
            case EQUAL:
                registers.push_back(reg1 == reg2);
                break;
            case NOT_EQUAL:
                registers.push_back(reg1 != reg2);
                break;
            default:
                std::cerr << "Error: Invalid comparison operation opcode" << std::endl;
        }
    } else {
        std::cerr << "Error: Unsupported types for comparison operation" << std::endl;
    }
}

void RegisterVM::HandleLoadConst(unsigned int constantIndex) {
    if (constantIndex >= program.size()) {
        std::cerr << "Error: Invalid constant index" << std::endl;
        return;
    }

    // Ensure the constants vector is large enough
    if (constantIndex >= constants.size()) {
        constants.resize(constantIndex + 1);
    }

    // Assign the value from the bytecode to the constants vector
    constants[constantIndex] = program[constantIndex].value;

    // Push the constant value onto the registers stack
    registers.push_back(constants[constantIndex]);
}

void RegisterVM::print() {
    if (registers.empty()) {
        std::cerr << "Error: No value to print" << std::endl;
        return;
    }

    auto value = registers.back();
    registers.pop_back();

    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int32_t>) {
            std::cout << "PRINT: " << arg << std::endl;
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << "PRINT: " << arg << std::endl;
        } else if constexpr (std::is_same_v<T, bool>) {
            std::cout << "PRINT: " << std::boolalpha << arg << std::endl;
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "PRINT: " << arg << std::endl;
        } else {
            std::cerr << "Unsupported type for PRINT operation" << std::endl;
        }
    }, value);
}

void RegisterVM::HandleDeclareVariable(unsigned int variableIndex) {
   if (variableIndex >= program.size()) {
        std::cerr << "Error: Invalid variable index" << std::endl;
        return;
    }

    // Check if the variable has already been declared
    if (variableIndex > variables.size()) {
        variables.resize(variableIndex + 1); // Resize the vector if needed
    } else {
        std::cerr << "Warning: Variable at index " << variableIndex << " already declared. Updating its value." << std::endl;
    }

    // Initialize the variable with the value provided in the bytecode
   variables[variableIndex-1] = program[variableIndex].value;
}

void RegisterVM::HandleLoadVariable(unsigned int variableIndex) {
    if (pc >= program.size()) {
        std::cerr << "Error: Invalid LOAD_VARIABLE instruction index" << std::endl;
        return;
    }

   // const auto& instruction = program[pc];
    if (variableIndex >= variables.size()) {
        std::cerr << "Error: Invalid variable index during loading" << std::endl;
        return;
    }

    registers.push_back(variables[variableIndex-1]);
}

void RegisterVM::HandleStoreVariable(unsigned int variableIndex) {
    if (variableIndex >= variables.size()) {
        std::cerr << "Error: Invalid variable index during storing" << std::endl;
        return;
    }

    variables[variableIndex-1] = registers.back();
    registers.pop_back();
}

void RegisterVM::HandleHalt() {
    std::cout << "Halt operation" << std::endl;
}
