#pragma once
#include "types.hh"

#include "scope.hh" // Include the new ScopeManager header
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
    std::optional<ValuePtr> value;
    TypePtr type;
};

class Variables
{
public:
    Variables(std::shared_ptr<TypeSystem> typeSystem)
        : typeSystem_(typeSystem)
        , scopeManager_()
    {}

    // Add a new variable to the current scope (local or global)
    int32_t addVariable(const std::string &name,
                        TypePtr type,
                        bool isGlobal = false,
                        std::optional<ValuePtr> defaultValue = std::nullopt)
    {
        static std::atomic<int32_t> nextMemoryLocation = 0;
        int32_t memoryLocation = nextMemoryLocation++;

        ValuePtr initialValue;
        if (defaultValue.has_value()) {
            if (!typeSystem_->isCompatibleType(*defaultValue.value(), type)) {
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

    // Check if a variable exists in any scope
    bool hasVariable(const std::string &name) const { return scopeManager_.exists(name); }

    // Get the memory location of a variable
    std::optional<int32_t> getVariableMemoryLocation(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return info->memoryLocation;
        }
        return std::nullopt;
    }

    // Get the type of a variable
    std::optional<TypePtr> getVariableType(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return info->type;
        }
        return std::nullopt;
    }

    // Get the value of a variable
    std::optional<ValuePtr> getVariableValue(const std::string &name) const
    {
        auto info = scopeManager_.get(name);
        if (info) {
            return info->value;
        }
        return std::nullopt;
    }

    // Set the value of a variable
    bool setVariableValue(const std::string &name, ValuePtr newValue)
    {
        auto info = scopeManager_.get(name);
        if (info) {
            if (typeSystem_->isCompatibleType(*newValue, info->type)) {
                info->value = newValue;
                return scopeManager_.update(name, *info);
            } else {
                throw std::runtime_error("Type mismatch when setting value for variable: " + name);
            }
        }
        return false;
    }

    // Enter a new local scope
    void enterScope() { scopeManager_.enterScope(); }

    // Exit the current scope
    void exitScope() { scopeManager_.exitScope(); }

private:
    std::shared_ptr<TypeSystem> typeSystem_;
    ScopeManager<VariableInfo> scopeManager_;

    ValuePtr createDefaultValueForType(TypePtr type)
    {
        Value defaultValue;
        defaultValue.type = type;

        switch (type->tag) {
        case TypeTag::Int:
        case TypeTag::Int32:
            defaultValue.data = 0;
            break;
        case TypeTag::Int8:
            defaultValue.data = int8_t(0);
            break;
        case TypeTag::Int16:
            defaultValue.data = int16_t(0);
            break;
        case TypeTag::Int64:
            defaultValue.data = int64_t(0);
            break;
        case TypeTag::UInt:
        case TypeTag::UInt32:
            defaultValue.data = uint32_t(0);
            break;
        case TypeTag::UInt8:
            defaultValue.data = uint8_t(0);
            break;
        case TypeTag::UInt16:
            defaultValue.data = uint16_t(0);
            break;
        case TypeTag::UInt64:
            defaultValue.data = uint64_t(0);
            break;
        case TypeTag::Float32:
            defaultValue.data = 0.0f;
            break;
        case TypeTag::Float64:
            defaultValue.data = 0.0;
            break;
        case TypeTag::String:
            defaultValue.data = std::string();
            break;
        case TypeTag::List:
            defaultValue.data = ListValue();
            break;
        case TypeTag::Dict:
            defaultValue.data = DictValue();
            break;
        case TypeTag::UserDefined:
            defaultValue.data = UserDefinedValue();
            break;
        default:
            throw std::runtime_error("Unsupported type for default value initialization");
        }

        return std::make_shared<Value>(defaultValue);
    }
};
