#pragma once

#include "function_types.hh"
#include "scope.hh"
#include "types.hh"
#include "variable.hh"
#include <functional>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

class Functions
{
public:
    Functions(std::shared_ptr<TypeSystem> typeSystem);

    void addFunction(const std::string &name,
                     const std::vector<ParameterInfo> &params,
                     TypePtr returnType,
                     int32_t startPC,
                     int32_t endPC,const std::vector<Instruction>& functionBody);

    bool isParameter(const std::string& funcName, const std::string& varName) const;

    void addBuiltinFunction(const std::string &name,
                            const std::vector<ParameterInfo> &params,
                            TypePtr returnType,
                            std::function<ValuePtr(const std::vector<ValuePtr> &)> implementation);

    std::optional<FunctionInfo> getFunction(const std::string &name) const;

    std::optional<FunctionInfo> getFunctionFromScope(
        const std::string &name, ScopeManager<FunctionInfo>::ScopeId scopeId) const;

    void updateFunctionEndPC(const std::string &name, int32_t endPC);

    ScopeManager<FunctionInfo>::ScopeId enterScope();
    void enterExistingScope(ScopeManager<FunctionInfo>::ScopeId scopeId);
    void exitScope();
    ScopeManager<FunctionInfo>::ScopeId getCurrentScopeId() const;
    bool isInCurrentScope(const std::string &name) const;
    bool hasFunction(const std::string &name) const;
    void validateFunctionCall(const std::string &name, const std::vector<ValuePtr> &arguments) const;
    ValuePtr executeBuiltin(const std::string &name, const std::vector<ValuePtr> &providedArgs) const;
    void pushParameterFrame(const std::string &functionName, const std::vector<ValuePtr> &args);
    void popParameterFrame();
    ValuePtr getParameter(const std::string &paramName) const;
    std::unordered_map<std::string, ValuePtr> getCurrentParameters() const;
    std::string getCurrentFunctionName() const;
    bool hasParameter(const std::string &paramName) const;
    size_t getParameterStackDepth() const;

    std::vector<ValuePtr> prepareArguments(const std::string &name,
                                           const std::vector<ValuePtr> &providedArgs) const;

    std::optional<std::vector<Instruction>> getFunctionBody(const std::string& name) const;

private:
    void validateParameters(const std::vector<ParameterInfo>& params, const std::string& functionName);

    std::shared_ptr<TypeSystem> typeSystem_;
    ScopeManager<FunctionInfo> scopeManager_;
    std::stack<std::shared_ptr<ParameterStackFrame>> parameterStack_;
    ScopeManager<FunctionInfo>::ScopeId currentScopeId_; // Track current scope ID
    Variables variable;
    mutable std::unordered_map<std::string, std::vector<Instruction>> optimizedBodies_;
};
