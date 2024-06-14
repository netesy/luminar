#ifndef CODEGEN_BACKEND_HH
#define CODEGEN_BACKEND_HH

#include "backend.hh"
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <variant>
#include <vector>

class CodegenBackend : public Backend {
public:
    CodegenBackend(const std::vector<Instruction>& program);
    void execute(const Instruction& instruction) override;
    void dumpRegisters() override;
    void run(const std::vector<Instruction> &program) override;
       unsigned int pc = 0;            // program counter

private:
    std::ofstream outFile;

    void generateUnaryOperation(const std::string& op);
    void generateBinaryOperation(const std::string& op);
    void generateComparisonOperation(const std::string& op);
    void generateLogicalOperation(const std::string& op);
    void generateLoadConst(int constantIndex);
    void generatePrint();
    void generateHalt();
    void generateDeclareVariable(int variableIndex);
    void generateLoadVariable(int variableIndex);
    void generateStoreVariable(int variableIndex);
    void generateWhileLoop();
    void generateParallel(int taskCount);
    void generateConcurrent(int taskCount);
};

#endif // CODEGEN_BACKEND_HH
