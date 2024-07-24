#ifndef BACKEND_HH
#define BACKEND_HH

#include "../instructions.hh"
#include "../types.hh"

//using Value = std::variant<int32_t, double, bool, std::string>;
//using Value = std::variant<int, double, bool, std::string, unsigned int>;

class Backend {
public:
    virtual ~Backend() = default;
    virtual void execute(const Instruction &instruction) = 0;
    virtual void dumpRegisters() = 0;
    virtual void run(const std::vector<Instruction> &program)  = 0;
};

#endif // BACKEND_HH
