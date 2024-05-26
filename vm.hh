#ifndef VM_HH
#define VM_HH

#include "opcodes.hh"
#include "parser.hh"
#include <iostream>
#include <variant>
#include <vector>

using Value = std::variant<int32_t, double, bool, std::string>;

class RegisterVM {
public:
    explicit RegisterVM(Parser &parser)
        : parser(parser)
    {
        program = parser.getBytecode();
    }

    void run();
    void execute(const Instruction &instruction);
    void dumpRegisters();

private:
    Parser &parser;
    unsigned int pc = 0;            // program counter
    std::vector<Value> registers;   // Registers for storing data
    std::vector<Value> constants;   // Constants for storing data
    std::vector<int32_t> variables; // Variables for storing data location
    std::vector<Instruction> program;

    void PerformUnaryOperation(int op);
    void PerformBinaryOperation(int op);
    void PerformLogicalOperation(int op);
    void PerformComparisonOperation(int op);
    void HandleLoadConst(unsigned int constantIndex);
    void HandleStoreValue(unsigned int constantIndex);
    void HandleDeclareVariable(unsigned int variableIndex);
    void HandleLoadVariable(unsigned int variableIndex);
    void HandleStoreVariable(unsigned int variableIndex);
    void HandleHalt();
    void print();
};

#endif // VM_HH
