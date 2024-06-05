#ifndef VM_HH
#define VM_HH

#include "parser.hh"
#include "backends/backend.hh"
#include <memory>

class VM {
public:
    explicit VM(Parser &parser, std::unique_ptr<Backend> backend)
        : parser(parser), backend(std::move(backend)) 
    {
        program = parser.getBytecode();
    }

    void run() {
        try {
            while (pc < program.size()) {
                const Instruction& instruction = program[pc];
                backend->execute(instruction);
                pc++;
            }
        } catch (const std::exception& ex) {
            std::cerr << "Exception occurred during VM execution: " << ex.what() << std::endl;
        }
    }

    void dumpRegisters() {
        backend->dumpRegisters();
    }

private:
    Parser &parser;
    unsigned int pc = 0; // program counter
    std::vector<Instruction> program;
    std::unique_ptr<Backend> backend;
};

#endif // VM_HH
