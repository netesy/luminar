#ifndef BACKEND_HH
#define BACKEND_HH

#include <variant>
#include <vector>
#include <string>
#include <thread>
#include <functional>
#include <mutex>
#include "../instructions.hh"

using Value = std::variant<int32_t, double, bool, std::string>;

class Backend {
public:
    virtual ~Backend() = default;
    virtual void execute(const Instruction &instruction) = 0;
    virtual void dumpRegisters() = 0;
};

#endif // BACKEND_HH
