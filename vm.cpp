#include "vm.hh"
#include "helper.hh"
const int REGISTER_COUNT = 64; // Define the maximum number of registers

void RegisterVM::run()
{
    std::cout << "Running the Vm loop" << std::endl;
    try {
        while (pc < program.size()) {
            const Instruction &instruction = program[pc]; // Fetch instruction at current PC
            switch (instruction.opcode) {
            case ADD:
            case SUBTRACT:
            case MULTIPLY:
            case DIVIDE:
            case MODULUS:
                PerformBinaryOperation(pc);
                pc++;
                break;
            case EQUAL:
            case NOT_EQUAL:
            case LESS_THAN:
            case LESS_THAN_OR_EQUAL:
            case GREATER_THAN:
            case GREATER_THAN_OR_EQUAL: {
                PerformComparisonOperation(pc);
                pc++;
                break;
            }
            case AND:
            case OR:
            case NOT:
                std::cout << "Logical" << std::endl;
                //                int reg1 = registers[--pc];
                //                int reg2 = registers[--pc];
                PerformLogicalOperation(pc);
                pc++;
                break;
            case LOAD_CONST:
                HandleLoadConst(pc);
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
                print();
                break;
            case DEFINE_FUNCTION:
            case INVOKE_FUNCTION:
            case RETURN_VALUE:
                break;
            case FOR_LOOP:
            case WHILE_LOOP:
                break;
            case ATTEMPT:
            case HANDLE:
                break;
            case DEFINE_CLASS:
            case CREATE_OBJECT:
            case METHOD_CALL:
                break;
            case OPEN_FILE:
            case WRITE_FILE:
            case CLOSE_FILE:
                break;
            case PARALLEL:
            case CONCURRENT:
            case ASYNC:
                break;
            case GENERIC_FUNCTION:
            case GENERIC_TYPE:
            case PATTERN_MATCH:
            case LOAD_VALUE:
                //            registers[pc++] = this->constants[pc++];
                registers[pc++] = std::get<int>(this->constants[pc++]);
                break;
            case STORE_VALUE:
                //            registers[pc++] = this->constants[pc++];
                //                registers[pc++] = std::get<int>(this->constants[pc++]);
                break;
            case LOAD_STR:
            case STORE_STR:
                //  registers[pc++] = std::get<int>(this->constants[pc++]);
                break;
            case CONCATENATE_STR:
                break;
            case ADD_ASSIGN:
            case SUB_ASSIGN:
            case NEGATE:
                break;
            case BOOLEAN:
                break;
            }
        }
    } catch (const std::exception &ex) {
        std::cerr << "Exception occurred during VM execution: " << ex.what() << std::endl;
    }
}

void RegisterVM::PerformBinaryOperation(int op)
{
    std::cout << "Arithmetic handling" << std::endl;
    int opcode = program[op].opcode; // Retrieve opcode from instruction

    // Ensure the operand registers exist
    if (registers.size() < 2) {
        // Handle invalid operand registers (e.g., throw an exception or return an error code)
        std::cerr << "Error: Invalid operand registers for binary operation" << std::endl;
        return;
    }

    // Get the operand registers
    int reg2 = registers.back(); // Get the second operand (reg2)
    registers.pop_back();
    int reg1 = registers.back(); // Get the first operand (reg1)
    registers.pop_back();

    std::cout << "Reg 1: " << reg1 << std::endl;
    std::cout << "Reg 2: " << reg2 << std::endl;
    std::cout << "Opcode: " << opcode << std::endl;

    try {
        switch (opcode) { // Get the binary opcode from the instruction
        case ADD:
            registers.push_back(reg1 + reg2);
            std::cout << "ADD Result: " << registers.back() << std::endl;
            break;
        case SUBTRACT:
            registers.push_back(reg1 - reg2);
            std::cout << "SUB Result: " << registers.back() << std::endl;
            break;
        case MULTIPLY:
            registers.push_back(reg1 * reg2);
            std::cout << "MUL Result: " << registers.back() << std::endl;
            break;
        case DIVIDE:
            if (reg2 == 0) {
                // Handle division by zero (e.g., throw an exception or set a register to an error value)
                std::cerr << "Error: Division by zero" << std::endl;
                return;
            }
            registers.push_back(reg1 / reg2);
            std::cout << "Divide Result: " << registers.back() << std::endl;
            break;
        case MODULUS:
            if (reg2 == 0) {
                // Handle modulo by zero (e.g., throw an exception or set a register to an error value)
                std::cerr << "Error: Modulo by zero" << std::endl;
                return;
            }
            registers.push_back(reg1 % reg2);
            std::cout << "Modulus Result: " << registers.back() << std::endl;
            break;
        default:
            // Handle invalid binary operation opcode
            // (e.g., throw an exception or log an error)
            std::cerr << "Error: Invalid binary operation opcode" << std::endl;
            break;
        }
    } catch (const std::exception &ex) {
        std::cerr << "Exception occurred during VM execution: " << ex.what() << std::endl;
    }
}

void RegisterVM::PerformLogicalOperation(int op)
{
    std::cout << "Logical Operation" << std::endl;
    int opcode = program[op].opcode; // Retrieve opcode from instruction
    int reg2 = op - 1;               // Get the second operand (reg2)
    int reg1 = op - 2;               // Get the first operand (reg1)
    switch (registers[--pc]) { // Get the logical opcode from the previous instruction
    case AND:
        registers[reg1] &= registers[reg2]; // Bitwise AND
        std::cout << "And Result: " << registers[reg1] << std::endl;
        break;
    case OR:
        registers[reg1] |= registers[reg2]; // Bitwise OR
        std::cout << " Or Result: " << registers[reg1] << std::endl;
        break;
    case NOT:
        registers[reg1] = ~registers[reg1]; // Bitwise NOT
        std::cout << " Not Result: " << registers[reg1] << std::endl;
        break;
    default:
        // Handle invalid logical operation opcode
        // (e.g., throw an exception or log an error)
        break;
    }
}

void RegisterVM::PerformComparisonOperation(int op)
{
    std::cout << "Comparison Operation" << std::endl;
    int opcode = program[op].opcode; // Retrieve opcode from instruction
    int reg2 = op - 1;               // Get the second operand (reg2)
    int reg1 = op - 2;               // Get the first operand (reg1)
    switch (opcode) {                // Get the comparison opcode from the previous instruction
    case GREATER_THAN:
        registers[reg1] = (registers[reg1] > registers[reg2]) ? 1 : 0;
        std::cout << "Result: " << registers[reg1] << std::endl;
        break;
    case LESS_THAN:
        registers[reg1] = (registers[reg1] < registers[reg2]) ? 1 : 0;
        std::cout << "Result: " << registers[reg1] << std::endl;
        break;
    case GREATER_THAN_OR_EQUAL:
        registers[reg1] = (registers[reg1] >= registers[reg2]) ? 1 : 0;
        std::cout << "Result: " << registers[reg1] << std::endl;
        break;
    case LESS_THAN_OR_EQUAL:
        registers[reg1] = (registers[reg1] <= registers[reg2]) ? 1 : 0;
        std::cout << "Result: " << registers[reg1] << std::endl;
        break;
    case EQUAL:
        registers[reg1] = (registers[reg1] == registers[reg2]) ? 1 : 0;
        std::cout << "Result: " << registers[reg1] << std::endl;
        break;
    case NOT_EQUAL:
        registers[reg1] = (registers[reg1] != registers[reg2]) ? 1 : 0;
        std::cout << "Result: " << registers[reg1] << std::endl;
        break;
    default:
        // Handle invalid comparison operation opcode
        // (e.g., throw an exception or log an error)
        break;
    }
}

void RegisterVM::HandleLoadConst(unsigned int constantIndex)
{
    // Ensure constants vector has enough space for constantIndex
    if (constantIndex >= constants.size()) {
        constants.resize(constantIndex + 1); // Resize the vector if necessary
    }
    std::cout << "Const Index: " << constantIndex << std::endl;

    // Get the constant value from the current instruction using the pc
    int constantValue = convertToInt(program[pc++].value);

    // Store the constant value in constants vector at constantIndex
    constants[constantIndex] = constantValue;

    // Set the register to the constant value
    registers.push_back(constantValue);

    std::cout << "Const Result: " << constantValue << std::endl;
}

void RegisterVM::HandleStoreValue(unsigned int constantIndex)
{
    std::cout << "Strings handling" << std::endl;
    if (constantIndex >= constants.size()) {
        // Handle invalid constant index (e.g., throw an exception or log an error)
    } else {
        // Determine the value to store from the registers or a specified source
        //        int valueToStore = ...; // Replace this placeholder with appropriate logic
        //        constants[constantIndex] = valueToStore;
        //   constants.push_back(this->program[constantIndex].value);
        pc++;
    }
}

void RegisterVM::HandleHalt()
{
    std::cout << "Halt operation" << std::endl;
    // Indicate that execution has finished
    // Perform any cleanup tasks (e.g., free resources, release locks)
    // Optionally, provide a method to retrieve final execution results
}

void RegisterVM::print()
{
    std::cout << "Print" << std::endl;
    if (this->constants[--pc].index() == 0) {
        // The constant is an integer
        std::cout << "PRINT: " << std::get<int>(this->constants[--pc]) << std::endl;
    } else if (this->constants[pc].index() == 1) {
        // The constant is a float
        std::cout << "PRINT: " << std::get<double>(this->constants[--pc]) << std::endl;
    } else if (this->constants[pc].index() == 2) {
        // The constant is a bool
        std::cout << "PRINT: " << std::get<bool>(this->constants[--pc]) << std::endl;
    } else if (this->constants[pc].index() == 3) {
        // The constant is a string
        std::cout << "PRINT: " << std::get<std::string>(this->constants[--pc]) << std::endl;
    } else {
        // Handle other types of constants (if necessary)
        // Modify this part according to your specific constant types
        std::cerr << "Unsupported constant type for PRINT operation" << std::endl;
        pc++;
    }
}
// ... Implement functions for other instruction types
