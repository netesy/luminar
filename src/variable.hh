//variable.hh
#pragma once
#include "scope.hh"
#include "types.hh"
#include <atomic>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

class TypeSystem; // Forward declaration

struct VariableInfo
{
    int32_t memoryLocation;
    bool isMutable;
    ValuePtr value;
    TypePtr type;
};

class Variables
{
public:
    Variables(std::shared_ptr<TypeSystem> typeSystem)
        : typeSystem_(typeSystem)
        , scopeManager_()
    {}

    int32_t addVariable(const std::string &name,
                        TypePtr type,
                        bool isGlobal = false,
                        std::optional<ValuePtr> defaultValue = std::nullopt)
    {
        static std::atomic<int32_t> nextMemoryLocation = 0;
        int32_t memoryLocation = nextMemoryLocation++;

        ValuePtr initialValue;
        if (defaultValue.has_value()) {
            if (!typeSystem_->checkType(defaultValue.value(), type)) {
                throw std::runtime_error(
                    "Default value type does not match declared type for variable: " + name);
            }
            initialValue = defaultValue.value();
        } else {
            initialValue = createDefaultValueForType(type);
        }

        VariableInfo info{memoryLocation,
                          true, // isMutable
                          initialValue,
                          type};

        if (isGlobal) {
            scopeManager_.addGlobal(name, info);
        } else {
            scopeManager_.add(name, info);
        }

        return memoryLocation;
    }

    bool hasVariable(const std::string &name) const { return scopeManager_.exists(name); }

    int32_t getVariableMemoryLocation(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return info->memoryLocation;
        }
        throw std::runtime_error("Variable not found: " + name);
    }

    TypePtr getVariableType(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return info->type;
        }
        throw std::runtime_error("Variable not found: " + name);
    }

    ValuePtr getVariableValue(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return info->value;
        }
        throw std::runtime_error("Variable not found: " + name);
    }

    void setVariableValue(const std::string &name, ValuePtr newValue)
    {
        auto info = scopeManager_.get(name);
        if (info) {
            if (typeSystem_->checkType(newValue, info->type)) {
                info->value = newValue;
                if (!scopeManager_.update(name, *info)) {
                    throw std::runtime_error("Failed to update variable: " + name);
                }
            } else {
                throw std::runtime_error("Type mismatch when setting value for variable: " + name);
            }
        } else {
            throw std::runtime_error("Variable not found: " + name);
        }
    }

    void enterScope() { scopeManager_.enterScope(); }
    void exitScope() { scopeManager_.exitScope(); }

private:
    std::shared_ptr<TypeSystem> typeSystem_;
    ScopeManager<VariableInfo> scopeManager_;

    ValuePtr createDefaultValueForType(TypePtr type)
    {

        auto defaultValue = std::make_shared<Value>();
        defaultValue->type = type;

        try {
            switch (type->tag) {
            case TypeTag::Bool:
                defaultValue->data = false;
                break;
            case TypeTag::Int:
            case TypeTag::Int32:
                defaultValue->data = int32_t(0);
                break;
            case TypeTag::Int8:
                defaultValue->data = int8_t(0);
                break;
            case TypeTag::Int16:
                defaultValue->data = int16_t(0);
                break;
            case TypeTag::Int64:
                defaultValue->data = int64_t(0);
                break;
            case TypeTag::UInt:
            case TypeTag::UInt32:
                defaultValue->data = uint32_t(0);
                break;
            case TypeTag::UInt8:
                defaultValue->data = uint8_t(0);
                break;
            case TypeTag::UInt16:
                defaultValue->data = uint16_t(0);
                break;
            case TypeTag::UInt64:
                defaultValue->data = uint64_t(0);
                break;
            case TypeTag::Float32:
                defaultValue->data = float(0.0);
                break;
            case TypeTag::Float64:
                defaultValue->data = double(0.0);
                break;
            case TypeTag::String:
                defaultValue->data = std::string();
                break;
            case TypeTag::List:
                defaultValue->data = ListValue();
                break;
            case TypeTag::Dict:
                defaultValue->data = DictValue();
                break;
            case TypeTag::UserDefined:
                defaultValue->data = UserDefinedValue();
                break;
            default:
                defaultValue->data = int32_t(0);
            }

        } catch (const std::exception &e) {
            std::cout << "Debug: Exception caught while creating default value: " << e.what()
                      << std::endl;
            throw;
        }

        return defaultValue;
    }
};
