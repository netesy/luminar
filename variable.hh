#pragma once
#include <atomic>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <stack>
using Scope = std::unordered_map<std::string, uint32_t>;

class Variables
{
public:
    Variables() {
        enterScope(); // Start with a global scope
    }

    // Add a new variable to the current scope
    uint32_t addVariable(const std::string &name)
    {
        if (currentScope().count(name) > 0) {
            throw std::runtime_error("Variable already declared in this scope: " + name);
        }
        
        // Use an atomic counter to ensure thread-safe memory allocation
        static std::atomic<uint32_t> nextMemoryLocation = 0;
        uint32_t memoryLocation = nextMemoryLocation++;
        currentScope()[name] = memoryLocation;
        return memoryLocation;
    }

    // Check if a variable exists in any scope
    bool hasVariable(const std::string &name) const {
        for (const auto &scope : scopeStack_) {
            if (scope.count(name) > 0) {
                return true;
            }
        }
        return false;
    }

    // Get the memory location of a variable
    uint32_t getVariableMemoryLocation(const std::string &name) const
    {
        for (const auto &scope : scopeStack_) {
            if (scope.count(name) > 0) {
                return scope.at(name);
            }
        }
        // Handle error: variable not found in any scope
        throw std::runtime_error("Variable not found: " + name);
    }

    // Enter a new scope
    void enterScope() {
        scopeStack_.push({});
    }

    // Exit the current scope
    void exitScope() {
        if (!scopeStack_.empty()) {
            scopeStack_.pop();
        } else {
            throw std::runtime_error("Mismatched scope exit");
        }
    }

private:
    std::stack<Scope> scopeStack_;

    Scope& currentScope() {
        return scopeStack_.top();
    }

    const Scope& currentScope() const {
        return scopeStack_.top();
    }
};
