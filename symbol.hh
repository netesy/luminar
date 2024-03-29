#pragma once
#include <atomic>
#include <stdexcept>
#include <string>
#include <unordered_map>

class SymbolTable
{
public:
    // Add a new variable to the symbol table and allocate memory
    uint32_t addVariable(const std::string &name)
    {
        // Use an atomic counter to ensure thread-safe memory allocation
        static std::atomic<uint32_t> nextMemoryLocation = 0;
        uint32_t memoryLocation = nextMemoryLocation++;
        symbolTable_[name] = memoryLocation;
        return memoryLocation;
    }

    // Check if a variable exists in the symbol table
    bool hasVariable(const std::string &name) const { return symbolTable_.count(name) > 0; }

    // Get the memory location of a variable
    uint32_t getVariableMemoryLocation(const std::string &name) const
    {
        if (hasVariable(name)) {
            return symbolTable_.at(name);
        } else {
            // Handle error: variable not found in symbol table
            throw std::runtime_error("Variable not found: " + name);
        }
    }

private:
    // Use an unordered_map to store variable names and their memory locations
    std::unordered_map<std::string, uint32_t> symbolTable_;
};
