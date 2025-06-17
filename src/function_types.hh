#pragma once

#include "instructions.hh"
#include "types.hh"
#include <atomic>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct ParameterInfo
{
    std::string name;
    TypePtr type;
    bool isOptional;
    ValuePtr defaultValue;

    ParameterInfo(const std::string &n, TypePtr t, bool opt = false, ValuePtr def = nullptr);
};

struct ParameterStackFrame
{
    std::string functionName;
    std::unordered_map<std::string, ValuePtr> parameters;

    ParameterStackFrame(const std::string &name);
};

struct FunctionInfo
{
    std::string name;
    std::vector<ParameterInfo> parameters;
    TypePtr returnType;
    int32_t startPC;
    mutable std::atomic<int32_t> endPC;
    bool isBuiltin;
    std::function<ValuePtr(const std::vector<ValuePtr> &)> nativeImpl;
    size_t requiredParamCount;
    std::vector<Instruction> functionBody;
    std::unordered_map<std::string, bool> variableMap;

    FunctionInfo();

    FunctionInfo(const std::string &n,
                 const std::vector<ParameterInfo> &params,
                 TypePtr ret,
                 int32_t start,
                 int32_t end,
                 size_t reqCount,
                 std::vector<Instruction> body);

    FunctionInfo(const std::string& n,
                const std::vector<ParameterInfo>& params,
                TypePtr ret,
                std::function<ValuePtr(const std::vector<ValuePtr>&)> impl,
                size_t reqCount);

    FunctionInfo(const FunctionInfo &other);
    FunctionInfo &operator=(const FunctionInfo &other);

private:
    void initVariableMap();
};
