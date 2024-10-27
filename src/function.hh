#pragma once
#include "scope.hh"
#include "types.hh"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// Enhanced parameter info to support optional parameters
struct ParameterInfo
{
    std::string name;
    TypePtr type;
    bool isOptional;
    ValuePtr defaultValue;

    ParameterInfo(const std::string &n, TypePtr t, bool opt = false, ValuePtr def = nullptr)
        : name(n)
        , type(t)
        , isOptional(opt)
        , defaultValue(def)
    {}
};

struct FunctionInfo
{
    std::string name;
    std::vector<ParameterInfo> parameters;
    TypePtr returnType;
    int32_t startPC;
    int32_t endPC;
    bool isBuiltin;
    std::function<ValuePtr(const std::vector<ValuePtr> &)> nativeImpl;
    size_t requiredParamCount;

    // Add default constructor
    FunctionInfo()
        : name("")
        , parameters()
        , returnType(nullptr)
        , startPC(-1)
        , endPC(-1)
        , isBuiltin(false)
        , nativeImpl(nullptr)
        , requiredParamCount(0)
    {}

    // Existing constructors...
    FunctionInfo(const std::string &n,
                 const std::vector<ParameterInfo> &params,
                 TypePtr ret,
                 int32_t start,
                 int32_t end,
                 size_t reqCount);

    FunctionInfo(const std::string &n,
                 const std::vector<ParameterInfo> &params,
                 TypePtr ret,
                 std::function<ValuePtr(const std::vector<ValuePtr> &)> impl,
                 size_t reqCount);
};

class Functions
{
public:
    Functions(std::shared_ptr<TypeSystem> typeSystem)
        : typeSystem_(typeSystem)
        , scopeManager_()
    {}

    // Add a new function definition with optional parameters
    void addFunction(const std::string &name,
                     const std::vector<ParameterInfo> &params,
                     TypePtr returnType,
                     int32_t startPC,
                     int32_t endPC)
    {
        // Count required parameters
        size_t requiredCount = std::count_if(params.begin(),
                                             params.end(),
                                             [](const ParameterInfo &param) {
                                                 return !param.isOptional;
                                             });

        // Validate optional parameters are after required ones
        bool foundOptional = false;
        for (const auto &param : params) {
            if (foundOptional && !param.isOptional) {
                throw std::runtime_error(
                    "Required parameters must come before optional parameters");
            }
            if (param.isOptional) {
                foundOptional = true;
                // Validate default value type
                if (param.defaultValue && !typeSystem_->checkType(param.defaultValue, param.type)) {
                    throw std::runtime_error("Default value type mismatch for parameter '"
                                             + param.name + "' in function '" + name + "'");
                }
            }
        }

        FunctionInfo info(name, params, returnType, startPC, endPC, requiredCount);

        if (scopeManager_.exists(name)) {
            throw std::runtime_error("Function already defined: " + name);
        }

        scopeManager_.add(name, info);
    }

    // Add a built-in function with optional parameters
    void addBuiltinFunction(const std::string &name,
                            const std::vector<ParameterInfo> &params,
                            TypePtr returnType,
                            std::function<ValuePtr(const std::vector<ValuePtr> &)> implementation)
    {
        size_t requiredCount = std::count_if(params.begin(),
                                             params.end(),
                                             [](const ParameterInfo &param) {
                                                 return !param.isOptional;
                                             });

        FunctionInfo info(name, params, returnType, implementation, requiredCount);

        if (scopeManager_.exists(name)) {
            throw std::runtime_error("Function already defined: " + name);
        }

        scopeManager_.add(name, info);
    }

    // Helper to prepare arguments with default values
    std::vector<ValuePtr> prepareArguments(const std::string &name,
                                           const std::vector<ValuePtr> &providedArgs) const
    {
        auto funcInfo = scopeManager_.get(name);
        if (!funcInfo) {
            throw std::runtime_error("Function not found: " + name);
        }

        // Check minimum required arguments
        if (providedArgs.size() < funcInfo->requiredParamCount) {
            throw std::runtime_error("Function '" + name + "' requires at least "
                                     + std::to_string(funcInfo->requiredParamCount)
                                     + " arguments, but got " + std::to_string(providedArgs.size()));
        }

        // Check maximum arguments
        if (providedArgs.size() > funcInfo->parameters.size()) {
            throw std::runtime_error("Function '" + name + "' accepts at most "
                                     + std::to_string(funcInfo->parameters.size())
                                     + " arguments, but got " + std::to_string(providedArgs.size()));
        }

        std::vector<ValuePtr> finalArgs;
        finalArgs.reserve(funcInfo->parameters.size());

        // Copy provided arguments
        for (size_t i = 0; i < providedArgs.size(); i++) {
            if (!typeSystem_->checkType(providedArgs[i], funcInfo->parameters[i].type)) {
                throw std::runtime_error("Type mismatch for argument " + std::to_string(i + 1)
                                         + " in function '" + name + "': expected "
                                         + funcInfo->parameters[i].type->toString() + " but got "
                                         + providedArgs[i]->type->toString());
            }
            finalArgs.push_back(providedArgs[i]);
        }

        // Fill in default values for missing optional parameters
        for (size_t i = providedArgs.size(); i < funcInfo->parameters.size(); i++) {
            const auto &param = funcInfo->parameters[i];
            if (!param.isOptional) {
                throw std::runtime_error("Internal error: required parameter after optional ones");
            }
            finalArgs.push_back(param.defaultValue);
        }

        return finalArgs;
    }

    // Validate function call
    void validateFunctionCall(const std::string &name, const std::vector<ValuePtr> &arguments) const
    {
        auto funcInfo = scopeManager_.get(name);
        if (!funcInfo) {
            throw std::runtime_error("Function not found: " + name);
        }

        if (arguments.size() < funcInfo->requiredParamCount) {
            throw std::runtime_error("Function '" + name + "' requires at least "
                                     + std::to_string(funcInfo->requiredParamCount)
                                     + " arguments, but got " + std::to_string(arguments.size()));
        }

        if (arguments.size() > funcInfo->parameters.size()) {
            throw std::runtime_error("Too many arguments for function '" + name + "'");
        }

        // Type check provided arguments
        for (size_t i = 0; i < arguments.size(); i++) {
            if (!typeSystem_->checkType(arguments[i], funcInfo->parameters[i].type)) {
                throw std::runtime_error("Type mismatch for argument " + std::to_string(i + 1)
                                         + " in function '" + name + "'");
            }
        }
    }

    // Execute a built-in function with optional parameters
    ValuePtr executeBuiltin(const std::string &name, const std::vector<ValuePtr> &providedArgs) const
    {
        auto funcInfo = scopeManager_.get(name);
        if (!funcInfo || !funcInfo->isBuiltin) {
            throw std::runtime_error("Built-in function not found: " + name);
        }

        // Prepare arguments with defaults
        auto finalArgs = prepareArguments(name, providedArgs);
        return funcInfo->nativeImpl(finalArgs);
    }

    bool hasFunction(const std::string &name) const { return scopeManager_.exists(name); }

    std::optional<FunctionInfo> getFunction(const std::string &name) const
    {
        if (auto funcInfo = scopeManager_.get(name)) {
            return *funcInfo; // Dereference the shared_ptr to return FunctionInfo
        }
        return std::nullopt;
    }

    // Add method to update function endPC
    void updateFunctionEndPC(const std::string &name, int32_t endPC)
    {
        auto funcInfo = scopeManager_.get(name);
        if (!funcInfo) {
            throw std::runtime_error("Function not found while updating endPC: " + name);
        }

        funcInfo->endPC = endPC;
    }

    // Enhanced function info retrieval with scope checking
    std::optional<FunctionInfo> getFunctionInScope(const std::string &name, size_t scopeDepth = 0)
    {
        // Calculate the actual scope depth considering the global scope
        size_t targetDepth = scopeManager_.getCurrentScopeDepth() - scopeDepth;
        auto scope = scopeManager_.getScopeAtDepth(targetDepth);

        if (scope && scope->count(name) > 0) {
            return scope->at(name);
        }
        return std::nullopt;
    }

    // Add method to check if a function is in the current scope
    bool isInCurrentScope(const std::string &name) const
    {
        return scopeManager_.getCurrentScope().count(name) > 0;
    }

    void enterScope() { scopeManager_.enterScope(); }
    void exitScope() { scopeManager_.exitScope(); }

private:
    std::shared_ptr<TypeSystem> typeSystem_;
    ScopeManager<FunctionInfo> scopeManager_;
};
