#include "opcodes.hh"
#include "parser.hh"

class RegisterVM
{
public:
    explicit RegisterVM(Parser &parser)
        : parser(parser)
    {
        std::cout << "Started Vm" << std::endl;
        program = parser.getBytecode();
    }

    void run();
    void dumpRegisters()
    {
        std::cout << "Registers:\n";
        for (size_t i = 0; i < registers.size(); ++i) {
            std::cout << "R" << i << ": " << registers[i] << "\n";
        }
    }

private:
    Parser &parser;
    unsigned int pc = 0;        // program counter
    std::vector<int> registers; // Registers for storing data
    std::vector<std::variant<int, double, bool, std::string>> constants;
    std::vector<Instruction> program;
    //int registers[14] = {0};
    //std::vector<int> constants;

    void PerformBinaryOperation(int op);
    void PerformLogicalOperation(int reg1, int reg2);
    void PerformComparisonOperation(int reg1, int reg2);
    void HandleLoadConst(unsigned int constantValue);
    void HandleStoreValue(unsigned int constantIndex);
    // ... Implement functions for other instruction types (LOAD_VARIABLE, etc.)
    void HandleHalt();
    // ... Implement functions for control flow instructions (JUMP, JUMP_IF_TRUE, etc.)
    void print();
};
