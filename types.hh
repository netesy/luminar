// types.hh
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

enum class TypeTag { Int, Float, String, List, Dict, Enum, Function, Any, UserDefined };

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
    std::variant<int32_t, double, std::string, ListValue, DictValue, UserDefinedValue> data;
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
        if (from->tag == TypeTag::Int && to->tag == TypeTag::Float)
            return true;
        if (from->tag == TypeTag::Float && to->tag == TypeTag::Int)
            return true;
        if (from->tag == TypeTag::Int && to->tag == TypeTag::String)
            return true;
        if (from->tag == TypeTag::Float && to->tag == TypeTag::String)
            return true;
        return false;
    }

public:
    const TypePtr INT_TYPE = std::make_shared<Type>(TypeTag::Int);
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

        if (targetType->tag == TypeTag::Int) {
            if (value.type->tag == TypeTag::Float) {
                convertedValue->data = static_cast<int32_t>(std::get<double>(value.data));
            } else if (value.type->tag == TypeTag::String) {
                convertedValue->data = std::stoi(std::get<std::string>(value.data));
            } else {
                throw std::runtime_error("Cannot convert to Int");
            }
        } else if (targetType->tag == TypeTag::Float) {
            if (value.type->tag == TypeTag::Int) {
                convertedValue->data = static_cast<double>(std::get<int32_t>(value.data));
            } else if (value.type->tag == TypeTag::String) {
                convertedValue->data = std::stod(std::get<std::string>(value.data));
            } else {
                throw std::runtime_error("Cannot convert to Float");
            }
        } else if (targetType->tag == TypeTag::String) {
            if (value.type->tag == TypeTag::Int) {
                convertedValue->data = std::to_string(std::get<int32_t>(value.data));
            } else if (value.type->tag == TypeTag::Float) {
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
