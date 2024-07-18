#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

enum class TypeTag {
    Int,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float,
    String,
    List,
    Dict,
    Enum,
    Function,
    Any,
    UserDefined
};

struct Type;
using TypePtr = std::shared_ptr<Type>;

struct ListType
{
    TypePtr elementType;
};

struct DictType
{
    TypePtr keyType;
    TypePtr valueType;
};

struct EnumType
{
    std::vector<std::string> values;
};

struct FunctionType
{
    std::vector<TypePtr> paramTypes;
    TypePtr returnType;
};

struct UserDefinedType
{
    std::string name;
    std::map<std::string, TypePtr> fields;
};

struct Type
{
    TypeTag tag;
    std::variant<std::monostate, ListType, DictType, EnumType, FunctionType, UserDefinedType> extra;

    Type(TypeTag t)
        : tag(t)
    {}
    Type(TypeTag t, ListType lt)
        : tag(t)
        , extra(lt)
    {}
    Type(TypeTag t, DictType dt)
        : tag(t)
        , extra(dt)
    {}
    Type(TypeTag t, EnumType et)
        : tag(t)
        , extra(et)
    {}
    Type(TypeTag t, FunctionType ft)
        : tag(t)
        , extra(ft)
    {}
    Type(TypeTag t, UserDefinedType udt)
        : tag(t)
        , extra(udt)
    {}
};

struct Value;
using ValuePtr = std::shared_ptr<Value>;

struct ListValue
{
    std::vector<ValuePtr> elements;
};

struct DictValue
{
    std::map<ValuePtr, ValuePtr> elements;
};

struct UserDefinedValue
{
    std::map<std::string, ValuePtr> fields;
};

struct Value
{
    TypePtr type;
    std::variant<int8_t,
                 int16_t,
                 int32_t,
                 int64_t,
                 uint8_t,
                 uint16_t,
                 uint32_t,
                 uint64_t,
                 double,
                 std::string,
                 ListValue,
                 DictValue,
                 UserDefinedValue>
        data;
};

class OverflowException : public std::runtime_error
{
public:
    OverflowException(const std::string &msg)
        : std::runtime_error(msg)
    {}
};

template<typename To, typename From>
To safe_cast(From value)
{
    To result = static_cast<To>(value);
    if (static_cast<From>(result) != value || (value > 0 && result < 0)
        || (value < 0 && result > 0)) {
        throw OverflowException("Overflow detected in integer conversion");
    }
    return result;
}

std::string typeTagToString(TypeTag tag)
{
    switch (tag) {
    case TypeTag::Int:
        return "Int";
    case TypeTag::Int8:
        return "Int8";
    case TypeTag::Int16:
        return "Int16";
    case TypeTag::Int32:
        return "Int32";
    case TypeTag::Int64:
        return "Int64";
    case TypeTag::UInt:
        return "UInt";
    case TypeTag::UInt8:
        return "UInt8";
    case TypeTag::UInt16:
        return "UInt16";
    case TypeTag::UInt32:
        return "UInt32";
    case TypeTag::UInt64:
        return "UInt64";
    case TypeTag::Float:
        return "Float";
    case TypeTag::String:
        return "String";
    case TypeTag::List:
        return "List";
    case TypeTag::Dict:
        return "Dict";
    case TypeTag::Enum:
        return "Enum";
    case TypeTag::Function:
        return "Function";
    case TypeTag::Any:
        return "Any";
    case TypeTag::UserDefined:
        return "UserDefined";
    default:
        return "Unknown";
    }
}

int getSizeInBits(TypeTag tag)
{
    switch (tag) {
    case TypeTag::Int8:
    case TypeTag::UInt8:
        return 8;
    case TypeTag::Int16:
    case TypeTag::UInt16:
        return 16;
    case TypeTag::Int:
    case TypeTag::UInt:
    case TypeTag::Int32:
    case TypeTag::UInt32:
        return 32;
    case TypeTag::Int64:
    case TypeTag::UInt64:
        return 64;
    default:
        return 0; // Unknown size
    }
}

class TypeSystem
{
private:
    std::map<std::string, TypePtr> userDefinedTypes;

    bool canConvert(TypePtr from, TypePtr to)
    {
        if (from == to)
            return true;
        if (to->tag == TypeTag::Any)
            return true;

        // Allow conversion between integer types, but warn about potential data loss
        if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
             || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64
             || from->tag == TypeTag::UInt || from->tag == TypeTag::UInt8
             || from->tag == TypeTag::UInt16 || from->tag == TypeTag::UInt32
             || from->tag == TypeTag::UInt64)
            && (to->tag == TypeTag::Int || to->tag == TypeTag::Int8 || to->tag == TypeTag::Int16
                || to->tag == TypeTag::Int32 || to->tag == TypeTag::Int64 || to->tag == TypeTag::UInt
                || to->tag == TypeTag::UInt8 || to->tag == TypeTag::UInt16
                || to->tag == TypeTag::UInt32 || to->tag == TypeTag::UInt64)) {
            if (getSizeInBits(from->tag) > getSizeInBits(to->tag)) {
                std::cout << "Warning: Potential data loss in conversion from "
                          << typeTagToString(from->tag) << " to " << typeTagToString(to->tag)
                          << std::endl;
            }
            return true;
        }

        // Allow conversion from any integer type to float
        if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
             || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64
             || from->tag == TypeTag::UInt || from->tag == TypeTag::UInt8
             || from->tag == TypeTag::UInt16 || from->tag == TypeTag::UInt32
             || from->tag == TypeTag::UInt64)
            && to->tag == TypeTag::Float)
            return true;

        // Allow conversion from any integer type to string
        if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
             || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64
             || from->tag == TypeTag::UInt || from->tag == TypeTag::UInt8
             || from->tag == TypeTag::UInt16 || from->tag == TypeTag::UInt32
             || from->tag == TypeTag::UInt64)
            && to->tag == TypeTag::String)
            return true;

        // Existing conversions
        if (from->tag == TypeTag::Float && to->tag == TypeTag::Int)
            return true;
        if (from->tag == TypeTag::Float && to->tag == TypeTag::String)
            return true;

        return false;
    }

public:
    const TypePtr INT_TYPE = std::make_shared<Type>(TypeTag::Int);
    const TypePtr INT8_TYPE = std::make_shared<Type>(TypeTag::Int8);
    const TypePtr INT16_TYPE = std::make_shared<Type>(TypeTag::Int16);
    const TypePtr INT32_TYPE = std::make_shared<Type>(TypeTag::Int32);
    const TypePtr INT64_TYPE = std::make_shared<Type>(TypeTag::Int64);
    const TypePtr UINT_TYPE = std::make_shared<Type>(TypeTag::UInt);
    const TypePtr UINT8_TYPE = std::make_shared<Type>(TypeTag::UInt8);
    const TypePtr UINT16_TYPE = std::make_shared<Type>(TypeTag::UInt16);
    const TypePtr UINT32_TYPE = std::make_shared<Type>(TypeTag::UInt32);
    const TypePtr UINT64_TYPE = std::make_shared<Type>(TypeTag::UInt64);
    const TypePtr FLOAT_TYPE = std::make_shared<Type>(TypeTag::Float);
    const TypePtr STRING_TYPE = std::make_shared<Type>(TypeTag::String);
    const TypePtr ANY_TYPE = std::make_shared<Type>(TypeTag::Any);

    TypePtr makeListType(TypePtr elementType)
    {
        return std::make_shared<Type>(TypeTag::List, ListType{elementType});
    }

    TypePtr makeDictType(TypePtr keyType, TypePtr valueType)
    {
        return std::make_shared<Type>(TypeTag::Dict, DictType{keyType, valueType});
    }

    TypePtr makeEnumType(const std::vector<std::string> &values)
    {
        return std::make_shared<Type>(TypeTag::Enum, EnumType{values});
    }

    TypePtr makeFunctionType(const std::vector<TypePtr> &paramTypes, TypePtr returnType)
    {
        return std::make_shared<Type>(TypeTag::Function, FunctionType{paramTypes, returnType});
    }

    TypePtr makeUserDefinedType(const std::string &name,
                                const std::map<std::string, TypePtr> &fields)
    {
        auto type = std::make_shared<Type>(TypeTag::UserDefined, UserDefinedType{name, fields});
        userDefinedTypes[name] = type;
        return type;
    }

    TypePtr getUserDefinedType(const std::string &name)
    {
        auto it = userDefinedTypes.find(name);
        if (it != userDefinedTypes.end()) {
            return it->second;
        }
        throw std::runtime_error("Undefined user type: " + name);
    }

    TypePtr inferType(const Value &value) { return value.type; }

    bool isCompatibleType(const Value &value, TypePtr type) { return canConvert(value.type, type); }

    ValuePtr convertValue(const Value &value, TypePtr targetType)
    {
        if (value.type == targetType) {
            return std::make_shared<Value>(value);
        }

        auto convertedValue = std::make_shared<Value>();
        convertedValue->type = targetType;

        // Helper function to convert between integer types
        auto convertInteger = [](auto fromValue, TypeTag toType)
            -> std::variant<int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t> {
            switch (toType) {
            case TypeTag::Int8:
                return safe_cast<int8_t>(fromValue);
            case TypeTag::Int16:
                return safe_cast<int16_t>(fromValue);
            case TypeTag::Int32:
                return safe_cast<int32_t>(fromValue);
            case TypeTag::Int64:
                return safe_cast<int64_t>(fromValue);
            case TypeTag::UInt8:
                return safe_cast<uint8_t>(fromValue);
            case TypeTag::UInt16:
                return safe_cast<uint16_t>(fromValue);
            case TypeTag::UInt32:
                return safe_cast<uint32_t>(fromValue);
            case TypeTag::UInt64:
                return safe_cast<uint64_t>(fromValue);
            default:
                throw std::runtime_error("Unsupported integer conversion");
            }
        };

        // Convert between integer types
        if ((value.type->tag == TypeTag::Int || value.type->tag == TypeTag::Int8
             || value.type->tag == TypeTag::Int16 || value.type->tag == TypeTag::Int32
             || value.type->tag == TypeTag::Int64 || value.type->tag == TypeTag::UInt
             || value.type->tag == TypeTag::UInt8 || value.type->tag == TypeTag::UInt16
             || value.type->tag == TypeTag::UInt32 || value.type->tag == TypeTag::UInt64)
            && (targetType->tag == TypeTag::Int || targetType->tag == TypeTag::Int8
                || targetType->tag == TypeTag::Int16 || targetType->tag == TypeTag::Int32
                || targetType->tag == TypeTag::Int64 || targetType->tag == TypeTag::UInt
                || targetType->tag == TypeTag::UInt8 || targetType->tag == TypeTag::UInt16
                || targetType->tag == TypeTag::UInt32 || targetType->tag == TypeTag::UInt64)) {
            try {
                std::visit(
                    [&](auto &&arg) { convertedValue->data = convertInteger(arg, targetType->tag); },
                    value.data);
            } catch (const OverflowException &e) {
                throw std::runtime_error("Overflow detected: " + std::string(e.what())
                                         + ". Conversion from " + typeTagToString(value.type->tag)
                                         + " to " + typeTagToString(targetType->tag)
                                         + " is not safe.");
            }

            // Check for potential data loss (e.g., converting from larger to smaller type)
            if (getSizeInBits(value.type->tag) > getSizeInBits(targetType->tag)) {
                std::cout << "Warning: Potential data loss in conversion from "
                          << typeTagToString(value.type->tag) << " to "
                          << typeTagToString(targetType->tag) << std::endl;
            }

            return convertedValue;
        }

        // Convert integer to float
        if ((value.type->tag == TypeTag::Int || value.type->tag == TypeTag::Int8
             || value.type->tag == TypeTag::Int16 || value.type->tag == TypeTag::Int32
             || value.type->tag == TypeTag::Int64 || value.type->tag == TypeTag::UInt
             || value.type->tag == TypeTag::UInt8 || value.type->tag == TypeTag::UInt16
             || value.type->tag == TypeTag::UInt32 || value.type->tag == TypeTag::UInt64)
            && targetType->tag == TypeTag::Float) {
            std::visit([&](auto &&arg) { convertedValue->data = static_cast<double>(arg); },
                       value.data);
            return convertedValue;
        }

        // Convert integer to string
        if ((value.type->tag == TypeTag::Int || value.type->tag == TypeTag::Int8
             || value.type->tag == TypeTag::Int16 || value.type->tag == TypeTag::Int32
             || value.type->tag == TypeTag::Int64 || value.type->tag == TypeTag::UInt
             || value.type->tag == TypeTag::UInt8 || value.type->tag == TypeTag::UInt16
             || value.type->tag == TypeTag::UInt32 || value.type->tag == TypeTag::UInt64)
            && targetType->tag == TypeTag::String) {
            std::visit([&](auto &&arg) { convertedValue->data = std::to_string(arg); }, value.data);
            return convertedValue;
        }

        // Existing conversions
        if (targetType->tag == TypeTag::Float && value.type->tag == TypeTag::Int) {
            convertedValue->data = static_cast<double>(std::get<int32_t>(value.data));
        } else if (targetType->tag == TypeTag::String) {
            if (value.type->tag == TypeTag::Float) {
                convertedValue->data = std::to_string(std::get<double>(value.data));
            } else {
                throw std::runtime_error("Cannot convert to String");
            }
        } else {
            throw std::runtime_error("Unsupported type conversion");
        }

        return convertedValue;
    }
};
