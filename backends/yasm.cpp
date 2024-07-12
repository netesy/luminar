#include "yasm.hh"

void YASMBackend::dumpRegisters()
{
    //    std::cout << "Stack:\n";
    //    std::stack<Value> tempStack = stack;
    //    while (!tempStack.empty()) {
    //        auto value = tempStack.top();
    //        tempStack.pop();
    //        std::visit([](const auto &val) { std::cout << val; }, value);
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

    //    std::cout << "Functions:\n";
    //    for (const auto &[name, _] : functions) {
    //        std::cout << "Function: " << name << "\n";
    //    }
}

void YASMBackend::run(const std::vector<Instruction> &program)
{
    outputFile.open("output.asm");

    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output file." << std::endl;
        return;
    }

    // Emit initial assembly setup
    emit("section .data");
    emit("format db '%d', 0");

    // Emit data section for strings
    for (const auto &[label, str] : stringTable) {
        emit(label + " db '" + str + "', 0");
    }

    emit("section .text");
    emit("global _start");
    emit("_start:");
    this->program = program; // Store the program in the instance
    try {
        pc = 0; // Reset program counter
        while (pc < program.size()) {
            const Instruction &instruction = program[pc];
            execute(instruction);
            pc++;
        }
    } catch (const std::exception &ex) {
        std::cerr << "Exception occurred during code generation: " << ex.what() << std::endl;
    }
    outputFile.close();
}

void YASMBackend::execute(const Instruction &instruction)
{
    //    for (const auto &instruction : program) {
    switch (instruction.opcode) {
    case NEGATE:
    case NOT:
        emitUnaryOperation(instruction);
        break;
    case ADD:
    case SUBTRACT:
    case MULTIPLY:
    case DIVIDE:
    case MODULUS:
        emitBinaryOperation(instruction);
        break;
    case EQUAL:
    case NOT_EQUAL:
    case LESS_THAN:
    case LESS_THAN_OR_EQUAL:
    case GREATER_THAN:
    case GREATER_THAN_OR_EQUAL:
        emitComparisonOperation(instruction);
        break;
    case AND:
    case OR:
        emitLogicalOperation(instruction);
        break;
    case LOAD_CONST:
    case LOAD_STR:
        emitLoadConst(instruction);
        break;
    case PRINT:
        emitPrint();
        break;
    case HALT:
        emitHalt();
        break;
    case DECLARE_VARIABLE:
        emitDeclareVariable(instruction);
        break;
    case LOAD_VARIABLE:
        emitLoadVariable(instruction);
        break;
    case STORE_VARIABLE:
        emitStoreVariable(instruction);
        break;
    case DEFINE_FUNCTION:
        emitDeclareFunction(instruction);
        break;
    case INVOKE_FUNCTION:
        emitCallFunction(instruction);
        break;
    case JUMP:
        emitJump(instruction);
        break;
    case JUMP_IF_FALSE:
        emitJumpZero(instruction);
        break;
    case CONCURRENT:
        emitHandleConcurrent(instruction);
        break;
    case PARALLEL:
        emitHandleParallel(instruction);
        break;
    default:
        std::cerr << "Unknown opcode." << std::endl;
    }
    //}
}

std::string YASMBackend::getUniqueLabel()
{
    return "L" + std::to_string(labelCounter++);
}

void YASMBackend::emit(const std::string &line)
{
    outputFile << line << std::endl;
    std::cout << line << std::endl;
}

void YASMBackend::emitUnaryOperation(const Instruction &instruction)
{
    if (instruction.opcode == NEGATE) {
        emit("pop rax");
        emit("neg rax");
        emit("push rax");
    } else if (instruction.opcode == NOT) {
        emit("pop rax");
        emit("not rax");
        emit("push rax");
    }
}

void YASMBackend::emitBinaryOperation(const Instruction &instruction)
{
    emit("pop rbx");
    emit("pop rax");
    switch (instruction.opcode) {
    case ADD:
        emit("add rax, rbx");
        break;
    case SUBTRACT:
        emit("sub rax, rbx");
        break;
    case MULTIPLY:
        emit("imul rax, rbx");
        break;
    case DIVIDE:
        emit("xor rdx, rdx");
        emit("idiv rbx");
        break;
    case MODULUS:
        emit("xor rdx, rdx");
        emit("idiv rbx");
        emit("mov rax, rdx");
        break;
    default:
        std::cerr << "Invalid binary operation opcode" << std::endl;
    }
    emit("push rax");
}

void YASMBackend::emitComparisonOperation(const Instruction &instruction)
{
    emit("pop rbx");
    emit("pop rax");
    emit("cmp rax, rbx");
    switch (instruction.opcode) {
    case EQUAL:
        emit("sete al");
        break;
    case NOT_EQUAL:
        emit("setne al");
        break;
    case LESS_THAN:
        emit("setl al");
        break;
    case LESS_THAN_OR_EQUAL:
        emit("setle al");
        break;
    case GREATER_THAN:
        emit("setg al");
        break;
    case GREATER_THAN_OR_EQUAL:
        emit("setge al");
        break;
    default:
        std::cerr << "Invalid comparison operation opcode" << std::endl;
    }
    emit("movzx rax, al");
    emit("push rax");
}

void YASMBackend::emitLogicalOperation(const Instruction &instruction)
{
    emit("pop rbx");
    emit("pop rax");
    switch (instruction.opcode) {
    case AND:
        emit("and rax, rbx");
        break;
    case OR:
        emit("or rax, rbx");
        break;
    default:
        std::cerr << "Invalid logical operation opcode" << std::endl;
    }
    emit("push rax");
}

void YASMBackend::emitLoadConst(const Instruction &instruction)
{
    if (std::holds_alternative<int32_t>(instruction.value)) {
        emit("mov rax, " + std::to_string(std::get<int32_t>(instruction.value)));
        emit("push rax");
        emit("\n");
    } else if (std::holds_alternative<double>(instruction.value)) {
        //        std::string dataLabel = "dbl" + std::to_string(dataLabelCounter++);
        //        emit(dataLabel + " dq " + std::to_string(std::get<double>(instruction.value)));
        //        emit("mov rax, [rip + " + dataLabel + "]");
        //        emit("push rax");

        // Define double constants in the .data section and load them
        std::string dataLabel = "dbl" + std::to_string(dataLabelCounter++);
        emit("section .data");
        emit(dataLabel + " dq " + std::to_string(std::get<double>(instruction.value)));
        emit("section .text");

        // Load the double constant
        emit("lea rax, [rel " + dataLabel + "]"); // Load address into rax
        emit("movsd xmm0, [rax]");                // Move double into xmm0 register
        emit("sub rsp, 8");                       // Allocate space on the stack
        emit("movsd [rsp], xmm0");                // Store double on the stack
        emit("\n");
    } else if (std::holds_alternative<bool>(instruction.value)) {
        bool value = std::get<bool>(instruction.value);
        emit("mov rax, " + std::to_string(value ? 1 : 0));
        emit("push rax");
        emit("\n");
    } else if (std::holds_alternative<std::string>(instruction.value)) {
        // Define string constants in the .data section and load them
        std::string str = std::get<std::string>(instruction.value);
        std::string label = "str" + std::to_string(dataLabelCounter++);
        emit("section .data");
        emit(label + " db '" + str + "'"); // Define string in data section
        emit(label + "len" + " db '" + " $-" + label);
        emit("section .text");

        // Load the address of the string
        emit("lea rax, [rel " + label + "]"); // Load address into rax
        emit("push rax");
        emit("\n");
    } else {
        //        std::string str = std::get<std::string>(instruction.value);
        //        std::string label = "str" + std::to_string(dataLabelCounter++);
        //        stringTable[label] = str;
        //        emit("mov rax, " + label);
        //        emit("push rax");
        // std::cerr << "Unsupported constant type" << std::endl;
        // Define string constants in the .data section and load them
        std::string str = std::get<std::string>(instruction.value);
        std::string label = "str" + std::to_string(dataLabelCounter++);
        emit("section .data");
        emit(label + " db '" + str + "'"); // Define string in data section
        emit(label + "len" + " db '" + " $-" + label);
        emit("section .text");

        // Load the address of the string
        emit("lea rax, [rel " + label + "]"); // Load address into rax
        emit("push rax");
        emit("\n");
    }
}

void YASMBackend::emitPrint()
{
    emit("section .text");
    emit("extern  printf");
    emit("pop rdi");
    emit("call printf"); // Assume there's a print function in assembly
}

void YASMBackend::emitHalt()
{
    emit("mov rax, 60");  // syscall number for exit
    emit("xor rdi, rdi"); // exit code 0
    emit("syscall");
}

void YASMBackend::emitDeclareVariable(const Instruction &instruction)
{
    unsigned int variableIndex = std::get<int32_t>(instruction.value);
    emit("sub rsp, 8"); // Allocate space on the stack for the variable
}

void YASMBackend::emitLoadVariable(const Instruction &instruction)
{
    unsigned int variableIndex = std::get<int32_t>(instruction.value);
    emit("mov rax, [rbp-" + std::to_string((variableIndex + 1) * 8) + "]");
    emit("push rax");
}

void YASMBackend::emitStoreVariable(const Instruction &instruction)
{
    unsigned int variableIndex = std::get<int32_t>(instruction.value);
    emit("pop rax");
    emit("mov [rbp-" + std::to_string((variableIndex + 1) * 8) + "], rax");
}

void YASMBackend::emitDeclareFunction(const Instruction &instruction)
{
    std::string functionName = std::get<std::string>(instruction.value);
    emit(functionName + ":");
}

void YASMBackend::emitCallFunction(const Instruction &instruction)
{
    std::string functionName = std::get<std::string>(instruction.value);
    emit("call " + functionName);
}

void YASMBackend::emitJump(const Instruction &instruction)
{
    int32_t offset = std::get<int32_t>(instruction.value);
    std::string label = getUniqueLabel();
    emit("jmp " + label);
    // In a real implementation, we need to handle labels and their positions
}

void YASMBackend::emitJumpZero(const Instruction &instruction)
{
    int32_t offset = std::get<int32_t>(instruction.value);
    std::string label = getUniqueLabel();
    emit("pop rax");
    emit("test rax, rax");
    emit("jz " + label);
    // In a real implementation, we need to handle labels and their positions
}

void YASMBackend::emitHandleConcurrent(const Instruction &instruction)
{
    std::string label = getUniqueLabel();
    emit("mov rdi, "
         + std::get<std::string>(instruction.value)); // Assumes value is a function name or address
    emit("call concurrent_function"); // Placeholder for the actual concurrent function handling
    emit(label + ":");
}

void YASMBackend::emitHandleParallel(const Instruction &instruction)
{
    std::string label = getUniqueLabel();
    emit("mov rdi, "
         + std::get<std::string>(instruction.value)); // Assumes value is a function name or address
    emit("call parallel_function"); // Placeholder for the actual parallel function handling
    emit(label + ":");
}
