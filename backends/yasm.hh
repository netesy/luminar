#include "backend.hh"
#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <variant>
#include <vector>

class YASMBackend : public Backend
{
public:
    explicit YASMBackend(std::vector<Instruction> &program)
        : program(program)
    {}
    void execute(const Instruction &instruction) override;
    void dumpRegisters() override;
    void run(const std::vector<Instruction> &program) override;

private:
    std::string getUniqueLabel();
    void emit(const std::string &line);
    void emitUnaryOperation(const Instruction &instruction);
    void emitBinaryOperation(const Instruction &instruction);
    void emitComparisonOperation(const Instruction &instruction);
    void emitLogicalOperation(const Instruction &instruction);
    void emitLoadConst(const Instruction &instruction);
    void emitPrint();
    void emitHalt();
    void emitDeclareVariable(const Instruction &instruction);
    void emitLoadVariable(const Instruction &instruction);
    void emitStoreVariable(const Instruction &instruction);
    void emitDeclareFunction(const Instruction &instruction);
    void emitCallFunction(const Instruction &instruction);
    void emitJump(const Instruction &instruction);
    void emitJumpZero(const Instruction &instruction);
    void emitHandleConcurrent(const Instruction &instruction);
    void emitHandleParallel(const Instruction &instruction);

    std::ofstream outputFile;
    std::vector<Instruction> &program;
    unsigned int pc = 0; // program counter
    size_t labelCounter = 0;
    size_t dataLabelCounter = 0;
    std::map<std::string, std::string> stringTable; // For storing string literals
};
