#include "vm.hh"

void RegisterVM::Run(const std::vector<Instruction> &program)
{
    this->program = program;
    while (pc < program.size()) {
        const Instruction &instruction = program[pc]; // Fetch instruction at current PC
        switch (instruction.opcode) {
        case ADD:
        case SUBTRACT:
        case MULTIPLY:
        case DIVIDE:
        case MODULUS:
            PerformBinaryOperation(registers[pc++], registers[pc++]);
            break;
        case EQUAL:
        case NOT_EQUAL:
        case LESS_THAN:
        case LESS_THAN_OR_EQUAL:
        case GREATER_THAN:
        case GREATER_THAN_OR_EQUAL:
            PerformComparisonOperation(registers[pc++], registers[pc++]);
            break;
        case AND:
        case OR:
        case NOT:
            PerformLogicalOperation(registers[pc++], registers[pc++]);
            break;
        case LOAD_CONST:
            HandleLoadConst(pc++);
            break;
        case JUMP:
        case JUMP_IF_TRUE:
        case JUMP_IF_FALSE:
            break;
        case DECLARE_VARIABLE:
        case LOAD_VARIABLE:
        case STORE_VARIABLE:
            break;
        case NOP:
        case HALT:
        case RETURN:
            return; // Halt execution
        case PRINT:
            // Handle printing the value in register or constant (modify for your needs)
            std::cout << "PRINT: " << registers[pc++] << std::endl;
        case DEFINE_FUNCTION:
        case INVOKE_FUNCTION:
        case RETURN_VALUE:
        case FOR_LOOP:
        case WHILE_LOOP:
        case ATTEMPT:
        case HANDLE:
        case DEFINE_CLASS:
        case CREATE_OBJECT:
        case METHOD_CALL:
        case OPEN_FILE:
        case WRITE_FILE:
        case CLOSE_FILE:
        case PARALLEL:
        case CONCURRENT:
        case ASYNC:
        case GENERIC_FUNCTION:
        case GENERIC_TYPE:
        case PATTERN_MATCH:
        case LOAD_VALUE:
            //            registers[pc++] = this->constants[pc++];
            registers[pc++] = std::get<int>(this->constants[pc++]);
            break;
        case STORE_VALUE:
            //            registers[pc++] = this->constants[pc++];
            registers[pc++] = std::get<int>(this->constants[pc++]);
            break;
        case LOAD_STR:
        case STORE_STR:
            registers[pc++] = std::get<int>(this->constants[pc++]);
            break;
        case CONCATENATE_STR:
            break;
        }
    }
}

void RegisterVM::PerformBinaryOperation(int reg1, int reg2)
{
    std::cout << "arithmetric handling" << std::endl;
    switch (registers[pc++]) { // Get the binary opcode from the next instruction
    case ADD:
        registers[reg1] += registers[reg2];
        std::cout << "Result: " << registers[reg1] << std::endl;
        break;
    case SUBTRACT:
        registers[reg1] -= registers[reg2];
        std::cout << "Result: " << registers[reg1] << std::endl;
        break;
    case MULTIPLY:
        registers[reg1] *= registers[reg2];
        std::cout << "Result: " << registers[reg1] << std::endl;
        break;
    case DIVIDE:
        if (registers[reg2] == 0) {
            // Handle division by zero (e.g., throw an exception or set a register to an error value)
        } else {
            registers[reg1] /= registers[reg2];
            std::cout << "Result: " << registers[reg1] << std::endl;
        }
        break;
    case MODULUS:
        if (registers[reg2] == 0) {
            // Handle modulo by zero (e.g., throw an exception or set a register to an error value)
        } else {
            registers[reg1] %= registers[reg2];
            std::cout << "Result: " << registers[reg1] << std::endl;
        }
        break;
    default:
        // Handle invalid binary operation opcode
        // (e.g., throw an exception or log an error)
        break;
    }
}

void RegisterVM::PerformLogicalOperation(int reg1, int reg2)
{
    switch (registers[pc++]) { // Get the logical opcode from the next instruction
    case AND:
        registers[reg1] &= registers[reg2]; // Bitwise AND
        break;
    case OR:
        registers[reg1] |= registers[reg2]; // Bitwise OR
        break;
    case NOT:
        registers[reg1] = ~registers[reg1]; // Bitwise NOT
        break;
    default:
        // Handle invalid logical operation opcode
        // (e.g., throw an exception or log an error)
        break;
    }
}

void RegisterVM::PerformComparisonOperation(int reg1, int reg2)
{
    switch (registers[pc++]) { // Get the comparison opcode from the next instruction
    case GREATER_THAN:
        registers[reg1] = (registers[reg1] > registers[reg2]) ? 1 : 0;
        break;
    case LESS_THAN:
        registers[reg1] = (registers[reg1] < registers[reg2]) ? 1 : 0;
        break;
    case GREATER_THAN_OR_EQUAL:
        registers[reg1] = (registers[reg1] >= registers[reg2]) ? 1 : 0;
        break;
    case LESS_THAN_OR_EQUAL:
        registers[reg1] = (registers[reg1] <= registers[reg2]) ? 1 : 0;
        break;
    case EQUAL:
        registers[reg1] = (registers[reg1] == registers[reg2]) ? 1 : 0;
        break;
    case NOT_EQUAL:
        registers[reg1] = (registers[reg1] != registers[reg2]) ? 1 : 0;
        break;
    default:
        // Handle invalid comparison operation opcode
        // (e.g., throw an exception or log an error)
        break;
    }
}
void RegisterVM::HandleLoadConst(int constantIndex)
{
    std::cout << "constants handling" << std::endl;

    if (constantIndex >= constants.size()) {
        // Handle invalid constant index (e.g., throw an exception or log an error)
    } else {
        //registers[pc++] = constants[constantIndex];
        registers[pc++] = std::get<int>(this->constants[pc++]);
        std::cout << "Result: " << registers[pc++] << std::endl;
    }
}

void RegisterVM::HandleStoreValue(int constantIndex)
{
    std::cout << "strings handling" << std::endl;
    if (constantIndex >= constants.size()) {
        // Handle invalid constant index (e.g., throw an exception or log an error)
    } else {
        // Determine the value to store from the registers or a specified source
        //        int valueToStore = ...; // Replace this placeholder with appropriate logic
        //        constants[constantIndex] = valueToStore;
        constants.push_back(this->program[constantIndex].value);
        pc++;
    }
}

void RegisterVM::HandleHalt()
{
    // Indicate that execution has finished
    // Perform any cleanup tasks (e.g., free resources, release locks)
    // Optionally, provide a method to retrieve final execution results
}

// ... Implement functions for other instruction types
