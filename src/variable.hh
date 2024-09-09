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
        std::cout << "Debug: Entering addVariable for " << name << std::endl;

        static std::atomic<int32_t> nextMemoryLocation = 0;
        int32_t memoryLocation = nextMemoryLocation++;
        std::cout << "Debug: Assigned memory location: " << memoryLocation << std::endl;

        ValuePtr initialValue;
        if (defaultValue.has_value()) {
            std::cout << "Debug: Default value provided" << std::endl;
            if (!typeSystem_->checkType(defaultValue.value(), type)) {
                std::cout << "Debug: Type mismatch for default value" << std::endl;
                throw std::runtime_error(
                    "Default value type does not match declared type for variable: " + name);
            }
            initialValue = defaultValue.value();
        } else {
            std::cout << "Debug: Creating default value for type" << std::endl;
            initialValue = createDefaultValueForType(type);
        }

        VariableInfo info{memoryLocation,
                          true, // isMutable
                          initialValue,
                          type};

        if (isGlobal) {
            std::cout << "Debug: Adding global variable" << std::endl;
            scopeManager_.addGlobal(name, info);
        } else {
            std::cout << "Debug: Adding local variable" << std::endl;
            scopeManager_.add(name, info);
        }

        std::cout << "Debug: Variable added successfully" << std::endl;
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
        std::cout << "Debug: Entering createDefaultValueForType" << std::endl;
        std::cout << "Debug: Type tag: " << static_cast<int>(type->tag) << std::endl;

        auto defaultValue = std::make_shared<Value>();
        defaultValue->type = type;

        try {
            switch (type->tag) {
            case TypeTag::Bool:
                std::cout << "Debug: Creating default bool value" << std::endl;
                defaultValue->data = false;
                break;
            case TypeTag::Int:
            case TypeTag::Int32:
                std::cout << "Debug: Creating default int32 value" << std::endl;
                defaultValue->data = int32_t(0);
                break;
            case TypeTag::Int8:
                std::cout << "Debug: Creating default int8 value" << std::endl;
                defaultValue->data = int8_t(0);
                break;
            case TypeTag::Int16:
                std::cout << "Debug: Creating default int16 value" << std::endl;
                defaultValue->data = int16_t(0);
                break;
            case TypeTag::Int64:
                std::cout << "Debug: Creating default int64 value" << std::endl;
                defaultValue->data = int64_t(0);
                break;
            case TypeTag::UInt:
            case TypeTag::UInt32:
                std::cout << "Debug: Creating default uint32 value" << std::endl;
                defaultValue->data = uint32_t(0);
                break;
            case TypeTag::UInt8:
                std::cout << "Debug: Creating default uint8 value" << std::endl;
                defaultValue->data = uint8_t(0);
                break;
            case TypeTag::UInt16:
                std::cout << "Debug: Creating default uint16 value" << std::endl;
                defaultValue->data = uint16_t(0);
                break;
            case TypeTag::UInt64:
                std::cout << "Debug: Creating default uint64 value" << std::endl;
                defaultValue->data = uint64_t(0);
                break;
            case TypeTag::Float32:
                std::cout << "Debug: Creating default float value" << std::endl;
                defaultValue->data = float(0.0);
                break;
            case TypeTag::Float64:
                std::cout << "Debug: Creating default double value" << std::endl;
                defaultValue->data = double(0.0);
                break;
            case TypeTag::String:
                std::cout << "Debug: Creating default string value" << std::endl;
                defaultValue->data = std::string();
                break;
            case TypeTag::List:
                std::cout << "Debug: Creating default list value" << std::endl;
                defaultValue->data = ListValue();
                break;
            case TypeTag::Dict:
                std::cout << "Debug: Creating default dict value" << std::endl;
                defaultValue->data = DictValue();
                break;
            case TypeTag::UserDefined:
                std::cout << "Debug: Creating default user-defined value" << std::endl;
                defaultValue->data = UserDefinedValue();
                break;
            default:
                std::cout << "Debug: Unsupported type, defaulting to int32" << std::endl;
                defaultValue->data = int32_t(0);
            }

            std::cout << "Debug: Default value created successfully" << std::endl;
        } catch (const std::exception &e) {
            std::cout << "Debug: Exception caught while creating default value: " << e.what()
                      << std::endl;
            throw;
        }

        return defaultValue;
    }
};
