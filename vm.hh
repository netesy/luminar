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
        backend->run(program);
    }

    void dumpRegisters() {
        backend->dumpRegisters();
    }

private:
    Parser &parser;
    std::vector<Instruction> program;
    std::unique_ptr<Backend> backend;
};

#endif // VM_HH
