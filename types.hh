// types.hh
#pragma once

#include <cstdint>
#include <functional>
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

        // Allow conversion between any integer types
        if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
             || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64
             || from->tag == TypeTag::UInt || from->tag == TypeTag::UInt8
             || from->tag == TypeTag::UInt16 || from->tag == TypeTag::UInt32
             || from->tag == TypeTag::UInt64)
            && (to->tag == TypeTag::Int || to->tag == TypeTag::Int8 || to->tag == TypeTag::Int16
                || to->tag == TypeTag::Int32 || to->tag == TypeTag::Int64 || to->tag == TypeTag::UInt
                || to->tag == TypeTag::UInt8 || to->tag == TypeTag::UInt16
                || to->tag == TypeTag::UInt32 || to->tag == TypeTag::UInt64))
            return true;

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

        if (from->tag == TypeTag::Float && to->tag == TypeTag::Int)
            return true;

        if (from->tag == TypeTag::Float && to->tag == TypeTag::String)
            return true;
        return false;
    }

public:
    const TypePtr INT8_TYPE = std::make_shared<Type>(TypeTag::Int8);
    const TypePtr INT16_TYPE = std::make_shared<Type>(TypeTag::Int16);
    const TypePtr INT32_TYPE = std::make_shared<Type>(TypeTag::Int32);
    const TypePtr INT64_TYPE = std::make_shared<Type>(TypeTag::Int64);
    const TypePtr UINT8_TYPE = std::make_shared<Type>(TypeTag::UInt8);
    const TypePtr UINT16_TYPE = std::make_shared<Type>(TypeTag::UInt16);
    const TypePtr UINT32_TYPE = std::make_shared<Type>(TypeTag::UInt32);
    const TypePtr UINT64_TYPE = std::make_shared<Type>(TypeTag::UInt64);
    const TypePtr INT_TYPE = std::make_shared<Type>(TypeTag::Int);
    const TypePtr UINT_TYPE = std::make_shared<Type>(TypeTag::UInt);
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
                return static_cast<int8_t>(fromValue);
            case TypeTag::Int16:
                return static_cast<int16_t>(fromValue);
            case TypeTag::Int32:
                return static_cast<int32_t>(fromValue);
            case TypeTag::Int64:
                return static_cast<int64_t>(fromValue);
            case TypeTag::UInt8:
                return static_cast<uint8_t>(fromValue);
            case TypeTag::UInt16:
                return static_cast<uint16_t>(fromValue);
            case TypeTag::UInt32:
                return static_cast<uint32_t>(fromValue);
            case TypeTag::UInt64:
                return static_cast<uint64_t>(fromValue);
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
            std::visit([&](auto &&
                               arg) { convertedValue->data = convertInteger(arg, targetType->tag); },
                       value.data);
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
