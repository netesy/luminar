#pragma once
#include <atomic>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <stack>
//using Scope = std::unordered_map<std::string, int32_t>;

//     struct VariableInfo {
//     uint32_t memoryLocation;
//     VariableType type;
//     bool isMutable;
// };

struct VariableInfo {
    int32_t memoryLocation;
    bool isMutable;
};

using Scope = std::unordered_map<std::string, VariableInfo>;
class Variables {
public:
  Variables() {
    // Add a global scope initially
    enterScope();
  }

  // Add a new variable to the current scope (local or global)
  int32_t addVariable(const std::string &name, bool isGlobal = true)
  {
    // Check if the variable already exists in the current scope
    if (!isGlobal && currentScope().count(name) > 0) {
        throw std::runtime_error("Variable already declared in this scope: " + name);
    }

    bool isMutable = true;
    static std::atomic<int32_t> nextMemoryLocation = 0;
    int32_t memoryLocation = nextMemoryLocation++;

    if (isGlobal) {
      // Add to the global scope (first element in the vector)
      scopeStack_.front()[name] = {memoryLocation, isMutable};
    } else {
      // Add to the current local scope
      currentScope()[name] = {memoryLocation, isMutable};
    }
    return memoryLocation;
  }

  // Check if a variable exists in any scope
  bool hasVariable(const std::string& name) const {
    for (const auto& scope : scopeStack_) {
      if (scope.count(name) > 0) {
        return true;
      }
    }
    return false;
  }

  // Get the memory location of a variable
  int32_t getVariableMemoryLocation(const std::string& name) const {
    for (const auto& scope : scopeStack_) {
      if (scope.count(name) > 0) {
        return scope.at(name).memoryLocation;
      }
    }
    return -1;
    //throw std::runtime_error("Variable not found: " + name);
  }

  // Get the mutability of a variable
  bool isVariableMutable(const std::string& name) const {
    for (const auto& scope : scopeStack_) {
      if (scope.count(name) > 0) {
        return scope.at(name).isMutable;
      }
    }
    throw std::runtime_error("Variable for mutablitiy check not found: " + name);
  }

  // Enter a new local scope
  void enterScope() {
    scopeStack_.push_back(Scope());
  }

  // Exit the current scope
  void exitScope() {
    if (scopeStack_.size() <= 1) {
      throw std::runtime_error("Mismatched scope exit (cannot exit global scope)");
    }
    scopeStack_.pop_back();
  }

private:
  std::vector<Scope> scopeStack_;  // Vector to store scopes (global first)

  Scope& currentScope() {
    return scopeStack_.back();
  }

  const Scope& currentScope() const {
    return scopeStack_.back();
  }
};
