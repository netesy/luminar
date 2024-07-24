#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

enum class TypeTag {
    Nil,
    Bool,
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
    Float32,
    Float64,
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

std::string typeTagToString(TypeTag tag)
{
    switch (tag) {
    case TypeTag::Nil:
        return "Nil";
    case TypeTag::Bool:
        return "Bool";
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
    case TypeTag::Float32:
        return "Float32";
    case TypeTag::Float64:
        return "Float64";
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
    case TypeTag::Float32:
        return 32;
    case TypeTag::Int64:
    case TypeTag::UInt64:
    case TypeTag::Float64:
        return 64;
    default:
        return 0;
    }
}

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
    std::variant<std::monostate,
                 bool,
                 int8_t,
                 int16_t,
                 int32_t,
                 int64_t,
                 uint8_t,
                 uint16_t,
                 uint32_t,
                 uint64_t,
                 double,
                 float,
                 std::string,
                 ListValue,
                 DictValue,
                 UserDefinedValue>
        data;
    friend std::ostream &operator<<(std::ostream &os, const Value &value);
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

class TypeSystem
{
private:
    std::map<std::string, TypePtr> userDefinedTypes;

    bool canConvert(TypePtr from, TypePtr to);

public:
    const TypePtr NIL_TYPE = std::make_shared<Type>(TypeTag::Nil);
    const TypePtr BOOL_TYPE = std::make_shared<Type>(TypeTag::Bool);
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
    const TypePtr FLOAT32_TYPE = std::make_shared<Type>(TypeTag::Float32);
    const TypePtr FLOAT64_TYPE = std::make_shared<Type>(TypeTag::Float64);
    const TypePtr STRING_TYPE = std::make_shared<Type>(TypeTag::String);
    const TypePtr ANY_TYPE = std::make_shared<Type>(TypeTag::Any);

    TypePtr getUserDefinedType(const std::string &name);
    void registerUserDefinedType(const std::string &name, TypePtr type);
    ValuePtr convert(const ValuePtr &value, TypePtr targetType);
    TypePtr inferType(const ValuePtr &value);
    bool checkType(const ValuePtr &value, TypePtr expectedType);
};

bool TypeSystem::canConvert(TypePtr from, TypePtr to)
{
    if (from == to)
        return true;
    if (to->tag == TypeTag::Any)
        return true;

    if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
         || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64 || from->tag == TypeTag::UInt
         || from->tag == TypeTag::UInt8 || from->tag == TypeTag::UInt16
         || from->tag == TypeTag::UInt32 || from->tag == TypeTag::UInt64)
        && (to->tag == TypeTag::Int || to->tag == TypeTag::Int8 || to->tag == TypeTag::Int16
            || to->tag == TypeTag::Int32 || to->tag == TypeTag::Int64 || to->tag == TypeTag::UInt
            || to->tag == TypeTag::UInt8 || to->tag == TypeTag::UInt16 || to->tag == TypeTag::UInt32
            || to->tag == TypeTag::UInt64)) {
        if (getSizeInBits(from->tag) > getSizeInBits(to->tag)) {
            std::cout << "Warning: Potential data loss in conversion from "
                      << typeTagToString(from->tag) << " to " << typeTagToString(to->tag)
                      << std::endl;
        }
        return true;
    }

    if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
         || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64 || from->tag == TypeTag::UInt
         || from->tag == TypeTag::UInt8 || from->tag == TypeTag::UInt16
         || from->tag == TypeTag::UInt32 || from->tag == TypeTag::UInt64)
        && (to->tag == TypeTag::Float32 || to->tag == TypeTag::Float64))
        return true;

    if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
         || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64 || from->tag == TypeTag::UInt
         || from->tag == TypeTag::UInt8 || from->tag == TypeTag::UInt16
         || from->tag == TypeTag::UInt32 || from->tag == TypeTag::UInt64)
        && to->tag == TypeTag::String)
        return true;

    if ((from->tag == TypeTag::Float32 || from->tag == TypeTag::Float64) && to->tag == TypeTag::Int)
        return true;

    if ((from->tag == TypeTag::Float32 || from->tag == TypeTag::Float64)
        && to->tag == TypeTag::String)
        return true;

    return false;
}

TypePtr TypeSystem::getUserDefinedType(const std::string &name)
{
    auto it = userDefinedTypes.find(name);
    if (it != userDefinedTypes.end())
        return it->second;
    else
        return nullptr;
}

void TypeSystem::registerUserDefinedType(const std::string &name, TypePtr type)
{
    userDefinedTypes[name] = type;
}

ValuePtr TypeSystem::convert(const ValuePtr &value, TypePtr targetType)
{
    if (value->type == targetType) {
        return value;
    }

    if (!canConvert(value->type, targetType)) {
        throw std::invalid_argument("Unsupported conversion");
    }

    ValuePtr newValue = std::make_shared<Value>();
    newValue->type = targetType;

    if (std::holds_alternative<int>(value->data)) {
        int intValue = std::get<int>(value->data);
        if (targetType == INT8_TYPE) {
            newValue->data = safe_cast<int8_t>(intValue);
        } else if (targetType == INT16_TYPE) {
            newValue->data = safe_cast<int16_t>(intValue);
        } else if (targetType == INT32_TYPE) {
            newValue->data = safe_cast<int32_t>(intValue);
        } else if (targetType == INT64_TYPE) {
            newValue->data = static_cast<int64_t>(intValue);
        } else if (targetType == UINT_TYPE) {
            newValue->data = safe_cast<unsigned int>(intValue);
        } else if (targetType == UINT8_TYPE) {
            newValue->data = safe_cast<uint8_t>(intValue);
        } else if (targetType == UINT16_TYPE) {
            newValue->data = safe_cast<uint16_t>(intValue);
        } else if (targetType == UINT32_TYPE) {
            newValue->data = safe_cast<uint32_t>(intValue);
        } else if (targetType == UINT64_TYPE) {
            newValue->data = safe_cast<uint64_t>(intValue);
        } else if (targetType == FLOAT32_TYPE) {
            newValue->data = static_cast<float>(intValue);
        } else if (targetType == FLOAT64_TYPE) {
            newValue->data = static_cast<double>(intValue);
        } else if (targetType == STRING_TYPE) {
            newValue->data = std::to_string(intValue);
        } else {
            throw std::invalid_argument("Unsupported conversion");
        }
    } else if (std::holds_alternative<float>(value->data)) {
        float floatValue = std::get<float>(value->data);
        if (targetType == INT_TYPE) {
            newValue->data = static_cast<int>(floatValue);
        } else if (targetType == STRING_TYPE) {
            newValue->data = std::to_string(floatValue);
        } else {
            throw std::invalid_argument("Unsupported conversion");
        }
    } else if (std::holds_alternative<double>(value->data)) {
        double doubleValue = std::get<double>(value->data);
        if (targetType == INT_TYPE) {
            newValue->data = static_cast<int>(doubleValue);
        } else if (targetType == STRING_TYPE) {
            newValue->data = std::to_string(doubleValue);
        } else {
            throw std::invalid_argument("Unsupported conversion");
        }
    } else if (std::holds_alternative<std::string>(value->data)) {
        std::string stringValue = std::get<std::string>(value->data);
        if (targetType == INT_TYPE) {
            newValue->data = std::stoi(stringValue);
        } else if (targetType == FLOAT32_TYPE) {
            newValue->data = std::stof(stringValue);
        } else if (targetType == FLOAT64_TYPE) {
            newValue->data = std::stod(stringValue);
        } else {
            throw std::invalid_argument("Unsupported conversion");
        }
    } else {
        throw std::invalid_argument("Unsupported conversion");
    }

    return newValue;
}

TypePtr TypeSystem::inferType(const ValuePtr &value)
{
    return value->type;
}

bool TypeSystem::checkType(const ValuePtr &value, TypePtr expectedType)
{
    return value->type == expectedType;
}

// Custom operator<< for std::monostate
std::ostream &operator<<(std::ostream &os, const std::monostate &)
{
    return os << "monostate";
}

// Custom operator<< for ListValue
std::ostream &operator<<(std::ostream &os, const ListValue &lv)
{
    os << "[";
    for (size_t i = 0; i < lv.elements.size(); ++i) {
        if (i > 0)
            os << ", ";
        os << lv.elements[i];
    }
    os << "]";
    return os;
}

// Custom operator<< for DictValue
std::ostream &operator<<(std::ostream &os, const DictValue &dv)
{
    os << "{";
    bool first = true;
    for (const auto &kv : dv.elements) {
        if (!first)
            os << ", ";
        first = false;
        os << kv.first << ": " << kv.second;
    }
    os << "}";
    return os;
}
std::ostream &operator<<(std::ostream &os, const Value &value)
{
    // Implement the output logic for Value
    // Example: os << value.someMember;
    return os;
}

// Custom operator<< for UserDefinedValue
std::ostream &operator<<(std::ostream &os, const UserDefinedValue &udv)
{
    os << "{";
    const auto &fields = udv.fields;
    bool first = true;
    for (const auto &field : fields) {
        if (!first)
            os << ", ";
        first = false;
        os << field.first << ": " << *(field.second);
    }
    os << "}";
    return os;
}
