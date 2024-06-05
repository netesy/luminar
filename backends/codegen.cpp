#include "codegen.hh"

CodegenBackend::CodegenBackend(const std::vector<Instruction>& program)
    : Backend(), outFile("generated_code.c") {
    outFile << "#include <stdio.h>\n";
    outFile << "#include <stdlib.h>\n";
    outFile << "#include <pthread.h>\n";
    outFile << "#define MAX_STACK_SIZE 1024\n";
    outFile << "int stack[MAX_STACK_SIZE];\n";
    outFile << "int top = -1;\n";
    outFile << "int constants[MAX_STACK_SIZE];\n";
    outFile << "int variables[MAX_STACK_SIZE];\n";
    outFile << "void* run_task(void* arg);\n\n";
    outFile << "int main() {\n";
}

void CodegenBackend::execute(const Instruction& instruction) {
    switch (instruction.opcode) {
        case NEGATE:
            generateUnaryOperation("NEGATE");
            break;
        case ADD:
            generateBinaryOperation("ADD");
            break;
        case SUBTRACT:
            generateBinaryOperation("SUBTRACT");
            break;
        case MULTIPLY:
            generateBinaryOperation("MULTIPLY");
            break;
        case DIVIDE:
            generateBinaryOperation("DIVIDE");
            break;
        case MODULUS:
            generateBinaryOperation("MODULUS");
            break;
        case EQUAL:
            generateComparisonOperation("EQUAL");
            break;
        case NOT_EQUAL:
            generateComparisonOperation("NOT_EQUAL");
            break;
        case LESS_THAN:
            generateComparisonOperation("LESS_THAN");
            break;
        case LESS_THAN_OR_EQUAL:
            generateComparisonOperation("LESS_THAN_OR_EQUAL");
            break;
        case GREATER_THAN:
            generateComparisonOperation("GREATER_THAN");
            break;
        case GREATER_THAN_OR_EQUAL:
            generateComparisonOperation("GREATER_THAN_OR_EQUAL");
            break;
        case AND:
            generateLogicalOperation("AND");
            break;
        case OR:
            generateLogicalOperation("OR");
            break;
        case NOT:
            generateUnaryOperation("NOT");
            break;
        case LOAD_CONST:
            generateLoadConst(pc);
            break;
        case PRINT:
            generatePrint();
            break;
        case HALT:
            generateHalt();
            break;
        case DECLARE_VARIABLE:
            generateDeclareVariable(pc);
            break;
        case LOAD_VARIABLE:
            generateLoadVariable(pc);
            break;
        case STORE_VARIABLE:
            generateStoreVariable(pc);
            break;
        case WHILE_LOOP:
            generateWhileLoop();
            break;
        case PARALLEL:
            generateParallel(pc);
            break;
        case CONCURRENT:
            generateConcurrent(pc);
            break;
        default:
            std::cerr << "Unknown opcode." << std::endl;
    }
}

void CodegenBackend::dumpRegisters() {
    // Not needed for code generation
}

void CodegenBackend::generateUnaryOperation(const std::string& op) {
    outFile << "// Unary Operation: " << op << "\n";
    outFile << "stack[top] = -stack[top];\n";
}

void CodegenBackend::generateBinaryOperation(const std::string& op) {
    outFile << "// Binary Operation: " << op << "\n";
    outFile << "stack[top-1] = stack[top-1] ";
    if (op == "ADD") outFile << "+";
    else if (op == "SUBTRACT") outFile << "-";
    else if (op == "MULTIPLY") outFile << "*";
    else if (op == "DIVIDE") outFile << "/";
    else if (op == "MODULUS") outFile << "%";
    outFile << " stack[top];\n";
    outFile << "top--;\n";
}

void CodegenBackend::generateComparisonOperation(const std::string& op) {
    outFile << "// Comparison Operation: " << op << "\n";
    outFile << "stack[top-1] = stack[top-1] ";
    if (op == "EQUAL") outFile << "==";
    else if (op == "NOT_EQUAL") outFile << "!=";
    else if (op == "LESS_THAN") outFile << "<";
    else if (op == "LESS_THAN_OR_EQUAL") outFile << "<=";
    else if (op == "GREATER_THAN") outFile << ">";
    else if (op == "GREATER_THAN_OR_EQUAL") outFile << ">=";
    outFile << " stack[top];\n";
    outFile << "top--;\n";
}

void CodegenBackend::generateLogicalOperation(const std::string& op) {
    outFile << "// Logical Operation: " << op << "\n";
    if (op == "NOT") {
        outFile << "stack[top] = !stack[top];\n";
    } else {
        outFile << "stack[top-1] = stack[top-1] ";
        if (op == "AND") outFile << "&&";
        else if (op == "OR") outFile << "||";
        outFile << " stack[top];\n";
        outFile << "top--;\n";
    }
}

void CodegenBackend::generateLoadConst(int constantIndex) {
    outFile << "// Load Constant\n";
    outFile << "stack[++top] = constants[" << constantIndex << "];\n";
}

void CodegenBackend::generatePrint() {
    outFile << "// Print\n";
    outFile << "printf(\"%d\\n\", stack[top--]);\n";
}

void CodegenBackend::generateHalt() {
    outFile << "// Halt\n";
    outFile << "exit(0);\n";
}

void CodegenBackend::generateDeclareVariable(int variableIndex) {
    outFile << "// Declare Variable\n";
    outFile << "variables[" << variableIndex << "] = 0;\n";
}

void CodegenBackend::generateLoadVariable(int variableIndex) {
    outFile << "// Load Variable\n";
    outFile << "stack[++top] = variables[" << variableIndex << "];\n";
}

void CodegenBackend::generateStoreVariable(int variableIndex) {
    outFile << "// Store Variable\n";
    outFile << "variables[" << variableIndex << "] = stack[top--];\n";
}

void CodegenBackend::generateWhileLoop() {
    // Note: This method will require additional logic to generate proper C loop structures
    // Placeholder for the logic to generate while loop
    outFile << "// While Loop\n";
    // Example:
    outFile << "while (condition) {\n";
    outFile << "    // Loop body\n";
    outFile << "}\n";
}

void CodegenBackend::generateParallel(int taskCount) {
    outFile << "// Parallel\n";
    outFile << "pthread_t threads[" << taskCount << "];\n";
    outFile << "for (int i = 0; i < " << taskCount << "; ++i) {\n";
    outFile << "    pthread_create(&threads[i], NULL, run_task, (void*)(intptr_t)i);\n";
    outFile << "}\n";
    outFile << "for (int i = 0; i < " << taskCount << "; ++i) {\n";
    outFile << "    pthread_join(threads[i], NULL);\n";
    outFile << "}\n";
}

void CodegenBackend::generateConcurrent(int taskCount) {
    outFile << "// Concurrent\n";
    outFile << "pthread_t threads[" << taskCount << "];\n";
    outFile << "for (int i = 0; i < " << taskCount << "; ++i) {\n";
    outFile << "    pthread_create(&threads[i], NULL, run_task, (void*)(intptr_t)i);\n";
    outFile << "}\n";
    outFile << "for (int i = 0; i < " << taskCount << "; ++i) {\n";
    outFile << "    pthread_join(threads[i], NULL);\n";
    outFile << "}\n";
}

extern "C" void* run_task(void* arg) {
    int task_id = (intptr_t)arg;
    // Implement the task execution logic here
    return NULL;
}


