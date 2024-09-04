#ifndef VM_HH
#define VM_HH

#include "backends/backend.hh"
#include "parser/algorithm.hh"
#include <memory>

class VM {
public:
    explicit VM(Algorithm &parser, std::unique_ptr<Backend> backend)
        : parser(parser)
        , backend(std::move(backend))
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
    Algorithm &parser;
    std::vector<Instruction> program;
    std::unique_ptr<Backend> backend;
};

#endif // VM_HH
