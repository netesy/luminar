#pragma once
#ifndef ALGORITHM_HH
#define ALGORITHM_HH

#include "../instructions.hh"

//using Value = std::variant<int32_t, double, bool, std::string>;
//using Value = std::variant<int, double, bool, std::string, unsigned int>;
using Bytecode = std::vector<Instruction>;

class Algorithm
{
public:
    virtual ~Algorithm() = default;
    virtual Bytecode parse() = 0;
    virtual std::string toString() const = 0;
    virtual std::vector<Instruction> getBytecode() const = 0;
};

#endif // ALGORITHM_HH
