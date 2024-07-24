//#include "codegen.hh"

//CodegenBackend::CodegenBackend(const std::string &outputFile) {
//    outFile.open(outputFile);
//    if (!outFile) {
//        throw std::runtime_error("Failed to open output file");
//    }
//    writeHeader();
//}

//CodegenBackend::~CodegenBackend() {
//    writeFooter();
//    if (outFile.is_open()) {
//        outFile.close();
//    }
//}

//void CodegenBackend::execute(const Instruction &instruction) {
//    switch (instruction.opcode) {
//        case LOAD_CONST:
//            handleLoadConst(instruction.value);
//            break;
//        case DECLARE_VARIABLE:
//            handleDeclareVariable(std::get<int32_t>(instruction.value));
//            break;
//        case LOAD_VARIABLE:
//            handleLoadVariable(std::get<int32_t>(instruction.value));
//            break;
//        case STORE_VARIABLE:
//            handleStoreVariable(std::get<int32_t>(instruction.value));
//            break;
//        case PRINT:
//            handlePrint();
//            break;
//        case HALT:
//            handleHalt();
//            break;
//        case ADD:
//        case SUBTRACT:
//        case MULTIPLY:
//        case DIVIDE:
//        case MODULUS:
//            handleBinaryOperation(instruction);
//            break;
//        case NEGATE:
//            handleUnaryOperation(instruction);
//            break;
//        case JUMP:
//            handleJump(std::get<int32_t>(instruction.value));
//            break;
//        case JUMP_IF_FALSE:
//            handleJumpZero(std::get<int32_t>(instruction.value));
//            break;
//        default:
//            outFile << "// Unknown opcode\n";
//    }
//}

//void CodegenBackend::dumpRegisters() {
//    // Not applicable for code generation, but could log state
//}

//void CodegenBackend::run(const std::vector<Instruction> &program) {
//    for (const auto &instruction : program) {
//        execute(instruction);
//    }
//}

//void CodegenBackend::handleLoadConst(const Value &constantValue) {
//    std::visit([this](auto &&val) {
//        outFile << "push(" << val << ");\n";
//    }, constantValue);
//}

//void CodegenBackend::handleDeclareVariable(unsigned int variableIndex) {
//    if (variableIndex >= variables.size()) {
//        variables.resize(variableIndex + 1);
//    }
//    outFile << "int var" << variableIndex << ";\n";
//}

//void CodegenBackend::handleLoadVariable(unsigned int variableIndex) {
//    if (variableIndex >= variables.size()) {
//        outFile << "// Error: Invalid variable index\n";
//        return;
//    }
//    outFile << "push(var" << variableIndex << ");\n";
//}

//void CodegenBackend::handleStoreVariable(unsigned int variableIndex) {
//    if (variableIndex >= variables.size()) {
//        variables.resize(variableIndex + 1);
//    }
//    outFile << "var" << variableIndex << " = pop();\n";
//}

//void CodegenBackend::handlePrint() {
//    outFile << "printf(\"%d\\n\", pop());\n";
//}

//void CodegenBackend::handleHalt() {
//    outFile << "exit(0);\n";
//}

//void CodegenBackend::handleBinaryOperation(const Instruction &instruction) {
//    outFile << "int b = pop();\n";
//    outFile << "int a = pop();\n";

//    switch (instruction.opcode) {
//        case ADD:
//            outFile << "push(a + b);\n";
//            break;
//        case SUBTRACT:
//            outFile << "push(a - b);\n";
//            break;
//        case MULTIPLY:
//            outFile << "push(a * b);\n";
//            break;
//        case DIVIDE:
//            outFile << "push(a / b);\n";
//            break;
//        case MODULUS:
//            outFile << "push(a % b);\n";
//            break;
//        default:
//            outFile << "// Error: Invalid binary operation\n";
//    }
//}

//void CodegenBackend::handleUnaryOperation(const Instruction &instruction) {
//    outFile << "int a = pop();\n";

//    switch (instruction.opcode) {
//        case NEGATE:
//            outFile << "push(-a);\n";
//            break;
//        default:
//            outFile << "// Error: Invalid unary operation\n";
//    }
//}

//void CodegenBackend::handleJump(int targetAddress) {
//    outFile << "goto label" << targetAddress << ";\n";
//}

//void CodegenBackend::handleJumpZero(int targetAddress) {
//    outFile << "if (pop() == 0) goto label" << targetAddress << ";\n";
//}

//void CodegenBackend::writeHeader() {
//    outFile << "#include <stdio.h>\n";
//    outFile << "#include <stdlib.h>\n\n";
//    outFile << "int stack[1024];\n";
//    outFile << "int sp = -1;\n\n";
//    outFile << "void push(int value) {\n";
//    outFile << "    stack[++sp] = value;\n";
//    outFile << "}\n\n";
//    outFile << "int pop() {\n";
//    outFile << "    return stack[sp--];\n";
//    outFile << "}\n\n";
//    outFile << "int main() {\n";
//}

//void CodegenBackend::writeFooter() {
//    outFile << "    return 0;\n";
//    outFile << "}\n";
//}
