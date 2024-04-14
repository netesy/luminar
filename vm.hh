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
            std::cout << "R-" << i << ": " << registers[i] << "\n";
        }
        for (size_t i = 0; i < constants.size(); ++i) {
            std::visit([&i](const auto &value) { std::cout << "C-" << i << ": " << value << "\n"; },
                       constants[i]);
        }
        //        for (size_t i = 0; i < constants.size(); ++i) {
        //            std::cout << "C-" << i << ": "
        //                      << std::get<std::variant<int, double, bool, std::string>>(constants[i])
        //                      << "\n";
        //        }
    }

private:
    Parser &parser;
    unsigned int pc = 0;        // program counter
    std::vector<int> registers; // Registers for storing data
    std::vector<std::variant<int, double, bool, std::string>> constants;
    std::vector<Instruction> program;

    void PerformBinaryOperation(int op);
    void PerformUnaryOperation(int op);
    void PerformLogicalOperation(int op);
    void PerformComparisonOperation(int op);
    void HandleLoadConst(unsigned int constantValue);
    void HandleStoreValue(unsigned int constantIndex);
    // ... Implement functions for other instruction types (LOAD_VARIABLE, etc.)
    void HandleHalt();
    // ... Implement functions for control flow instructions (JUMP, JUMP_IF_TRUE, etc.)
    void print();
};
