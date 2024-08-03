// function.hh
#pragma once
#include "scope.hh"
#include "types.hh"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class TypeSystem; // Forward declaration

struct ParameterInfo
{
    std::string name;
    TypePtr type;
};

struct FunctionInfo
{
    std::vector<ParameterInfo> parameters;
    TypePtr returnType;
    int32_t address; // Memory address or index of the function
};

class Functions
{
public:
    Functions(std::shared_ptr<TypeSystem> typeSystem)
        : typeSystem_(typeSystem)
        , scopeManager_()
    {}

    int32_t addFunction(const std::string &name,
                        const std::vector<ParameterInfo> &parameters,
                        TypePtr returnType)
    {
        static int32_t nextFunctionAddress = 0;
        int32_t functionAddress = nextFunctionAddress++;

        FunctionInfo info{parameters, returnType, functionAddress};

        if (!scopeManager_.add(name, info)) {
            throw std::runtime_error("Function already exists: " + name);
        }

        return functionAddress;
    }

    bool hasFunction(const std::string &name) const { return scopeManager_.exists(name); }

    FunctionInfo getFunctionInfo(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return *info;
        }
        throw std::runtime_error("Function not found: " + name);
    }

    int32_t getFunctionAddress(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return info->address;
        }
        throw std::runtime_error("Function not found: " + name);
    }

    TypePtr getFunctionReturnType(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return info->returnType;
        }
        throw std::runtime_error("Function not found: " + name);
    }

    std::vector<ParameterInfo> getFunctionParameters(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return info->parameters;
        }
        throw std::runtime_error("Function not found: " + name);
    }

    void enterScope() { scopeManager_.enterScope(); }
    void exitScope() { scopeManager_.exitScope(); }

private:
    std::shared_ptr<TypeSystem> typeSystem_;
    ScopeManager<FunctionInfo> scopeManager_;
};
