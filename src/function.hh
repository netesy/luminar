#pragma once
#include "scope.hh"
#include "types.hh"
#include <functional>
#include <memory>
#include <optional>
#include <stack>
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

// New structure to hold parameter state
struct ParameterStackFrame
{
    std::string functionName;
    std::unordered_map<std::string, ValuePtr> parameters;

    ParameterStackFrame(const std::string &name)
        : functionName(name)
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

    FunctionInfo(const std::string &n,
                 const std::vector<ParameterInfo> &params,
                 TypePtr ret,
                 int32_t start,
                 int32_t end,
                 size_t reqCount)
        : name(n)
        , parameters(params)
        , returnType(ret)
        , startPC(start)
        , endPC(end)
        , isBuiltin(false)
        , nativeImpl(nullptr)
        , requiredParamCount(reqCount)
    {}

    FunctionInfo(const std::string &n,
                 const std::vector<ParameterInfo> &params,
                 TypePtr ret,
                 std::function<ValuePtr(const std::vector<ValuePtr> &)> impl,
                 size_t reqCount)
        : name(n)
        , parameters(params)
        , returnType(ret)
        , startPC(-1)
        , endPC(-1)
        , isBuiltin(true)
        , nativeImpl(impl)
        , requiredParamCount(reqCount)
    {}
};

class Functions
{
public:
    Functions(std::shared_ptr<TypeSystem> typeSystem)
        : typeSystem_(typeSystem)
        , scopeManager_()
    {}

    // Parameter stack management methods
    void pushParameterFrame(const std::string &functionName, const std::vector<ValuePtr> &args)
    {
        auto funcInfo = scopeManager_.get(functionName);
        if (!funcInfo) {
            throw std::runtime_error("Cannot push parameters for undefined function: "
                                     + functionName);
        }

        auto frame = std::make_shared<ParameterStackFrame>(functionName);

        // Map arguments to parameter names
        for (size_t i = 0; i < args.size(); ++i) {
            frame->parameters[funcInfo->parameters[i].name] = args[i];
        }

        // Add default values for remaining optional parameters
        for (size_t i = args.size(); i < funcInfo->parameters.size(); ++i) {
            const auto &param = funcInfo->parameters[i];
            if (param.isOptional) {
                frame->parameters[param.name] = param.defaultValue;
            }
        }

        parameterStack_.push(frame);
    }

    void popParameterFrame()
    {
        if (parameterStack_.empty()) {
            throw std::runtime_error("Cannot pop parameter frame: stack is empty");
        }
        parameterStack_.pop();
    }

    // Get parameter value from current frame
    ValuePtr getParameter(const std::string &paramName) const
    {
        if (parameterStack_.empty()) {
            throw std::runtime_error("No active function call");
        }

        const auto &currentFrame = parameterStack_.top();
        auto it = currentFrame->parameters.find(paramName);
        if (it == currentFrame->parameters.end()) {
            throw std::runtime_error("Parameter not found: " + paramName + " in function "
                                     + currentFrame->functionName);
        }
        return it->second;
    }

    // Get all parameters from current frame
    std::unordered_map<std::string, ValuePtr> getCurrentParameters() const
    {
        if (parameterStack_.empty()) {
            throw std::runtime_error("No active function call");
        }
        return parameterStack_.top()->parameters;
    }

    // Get current function name
    std::string getCurrentFunctionName() const
    {
        if (parameterStack_.empty()) {
            throw std::runtime_error("No active function call");
        }
        return parameterStack_.top()->functionName;
    }

    // Check if a parameter exists in current frame
    bool hasParameter(const std::string &paramName) const
    {
        if (parameterStack_.empty()) {
            return false;
        }
        return parameterStack_.top()->parameters.count(paramName) > 0;
    }

    // Get parameter stack depth
    size_t getParameterStackDepth() const { return parameterStack_.size(); }

    // Modified function call preparation to use parameter stack
    std::vector<ValuePtr> prepareArguments(const std::string &name,
                                           const std::vector<ValuePtr> &providedArgs) const
    {
        auto funcInfo = scopeManager_.get(name);
        if (!funcInfo) {
            throw std::runtime_error("Function not found: " + name);
        }

        validateFunctionCall(name, providedArgs);

        std::vector<ValuePtr> finalArgs;
        finalArgs.reserve(funcInfo->parameters.size());

        // Copy provided arguments
        for (size_t i = 0; i < providedArgs.size(); i++) {
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

    // Existing methods remain largely unchanged...
    void addFunction(const std::string &name,
                     const std::vector<ParameterInfo> &params,
                     TypePtr returnType,
                     int32_t startPC,
                     int32_t endPC)
    {
        size_t requiredCount = std::count_if(params.begin(),
                                             params.end(),
                                             [](const ParameterInfo &param) {
                                                 return !param.isOptional;
                                             });

        bool foundOptional = false;
        for (const auto &param : params) {
            if (foundOptional && !param.isOptional) {
                throw std::runtime_error(
                    "Required parameters must come before optional parameters");
            }
            if (param.isOptional) {
                foundOptional = true;
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

    // Other existing methods...
    void validateFunctionCall(const std::string &name, const std::vector<ValuePtr> &arguments) const
    {
        auto funcInfo = scopeManager_.get(name);
        if (!funcInfo) {
            throw std::runtime_error("Function not found: " + name);
        }

        // Check if the correct number of required arguments are provided
        if (arguments.size() < funcInfo->requiredParamCount
            || arguments.size() > funcInfo->parameters.size()) {
            throw std::runtime_error("Incorrect number of arguments for function: " + name);
        }

        // Validate each argument type
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (!typeSystem_->isCompatible(arguments[i]->type, funcInfo->parameters[i].type)) {
                throw std::runtime_error("Argument type mismatch for parameter '"
                                         + funcInfo->parameters[i].name + "' in function '" + name
                                         + "'");
            }
        }
    }

    ValuePtr executeBuiltin(const std::string &name, const std::vector<ValuePtr> &providedArgs) const
    {
        auto funcInfo = scopeManager_.get(name);
        if (!funcInfo || !funcInfo->isBuiltin) {
            throw std::runtime_error("Builtin function not found or is not a builtin: " + name);
        }

        return funcInfo->nativeImpl(providedArgs); // Execute the native implementation
    }
    bool hasFunction(const std::string &name) const { return scopeManager_.exists(name); }
    std::optional<FunctionInfo> getFunction(const std::string &name) const
    {
        if (auto funcInfo = scopeManager_.get(name)) {
            return *funcInfo;
        }
        return std::nullopt;
    }
    void updateFunctionEndPC(const std::string &name, int32_t endPC)
    {
        auto funcInfo = scopeManager_.get(name);
        if (!funcInfo) {
            throw std::runtime_error("Function not found: " + name);
        }

        funcInfo->endPC = endPC; // Update the function's end PC in the scope manager
    }
    // std::optional<FunctionInfo> getFunctionInScope(const std::string &name, size_t scopeDepth = 0)
    // {
    //     return scopeManager_..
    //     // Modify this depending on how `getInScope` is implemented
    // }
    bool isInCurrentScope(const std::string &name) const
    {
        return scopeManager_.existsInCurrentScope(name);
    }
    void enterScope() { scopeManager_.enterScope(); }
    void exitScope() { scopeManager_.exitScope(); }

private:
    std::shared_ptr<TypeSystem> typeSystem_;
    ScopeManager<FunctionInfo> scopeManager_;
    std::stack<std::shared_ptr<ParameterStackFrame>> parameterStack_;
};
