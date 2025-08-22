#ifndef VM_HH
#define VM_HH

#include "backends/backend.hh"
#include "parser/algorithm.hh"
#include <memory>

class VM {
public:
    explicit VM(Bytecode bytecode, std::unique_ptr<Backend> backend)
        : program(std::move(bytecode))
        , backend(std::move(backend))
    {}

    void run() {
        backend->run(program);
    }

    void dumpRegisters() {
        backend->dumpRegisters();
    }

private:
    std::vector<Instruction> program;
    std::unique_ptr<Backend> backend;
};

#endif // VM_HH
