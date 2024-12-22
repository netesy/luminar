//value.hh
#pragma once

#include <algorithm>
#include <array>
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
    Sum,
    Union,
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

    void addVariant(const std::string& name) {
        if (std::find(values.begin(), values.end(), name) != values.end()) {
            throw std::runtime_error("Enum variant already exists: " + name);
        }
        values.push_back(name);
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "Enum(";
        for (auto it = values.begin(); it != values.end(); ++it) {
            if (it != values.begin()) oss << ", ";
            oss << *it;
        }
        oss << ")";
        return oss.str();
    }
};

struct FunctionType
{
    std::vector<TypePtr> paramTypes;
    TypePtr returnType;
};

struct UserDefinedType
{
    std::string name;
    std::vector<std::pair<std::string, std::map<std::string, TypePtr>>> fields;
};

struct SumType
{
    std::vector<TypePtr> variants;
};

struct UnionType
{
    std::vector<TypePtr> types;
};

struct Type
{
    TypeTag tag;
    std::variant<std::monostate, ListType, DictType, EnumType, FunctionType, SumType, UnionType, UserDefinedType>
        extra;

    Type(TypeTag t)
        : tag(t)
    {}
    Type(TypeTag t,
         const std::variant<std::monostate,
                            ListType,
                            DictType,
                            EnumType,
                            FunctionType,
                            SumType,
                            UnionType,
                            UserDefinedType> &ex)
        : tag(t)
        , extra(ex)
    {}

    std::string toString() const
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
        case TypeTag::Sum:
            return "Sum";
        case TypeTag::Union:
            return "Union";
        case TypeTag::UserDefined:
            return "UserDefined";
        default:
            return "Unknown";
        }
    }

    bool operator==(const Type &other) const { return tag == other.tag; }
    bool operator!=(const Type &other) const { return !(*this == other); }
};

constexpr int getSizeInBits(TypeTag tag)
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

template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

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
    std::string variantName;
    std::map<std::string, ValuePtr> fields;
};

struct SumValue
{
    size_t activeVariant;
    ValuePtr value;
};

struct EnumValue {
    std::string variantName;
    ValuePtr associatedValue;

    EnumValue() = default;

    EnumValue(const std::string& name, const TypePtr& enumType, ValuePtr value = nullptr)
        : variantName(name), associatedValue(value) {
        // Validate variant name
        const auto* enumTypeDetails = std::get_if<EnumType>(&enumType->extra);
        if (!enumTypeDetails) {
            throw std::runtime_error("Invalid enum type");
        }

        auto it = std::find(enumTypeDetails->values.begin(), enumTypeDetails->values.end(), name);
        if (it == enumTypeDetails->values.end()) {
            throw std::runtime_error("Unknown enum variant: " + name);
        }
    }

    static ValuePtr create(const std::string& variantName, const TypePtr& enumType, ValuePtr associatedValue = nullptr);

    std::string toString() const {
        // if (associatedValue) {
        //     return "Enum(" + variantName + ", " + associatedValue->toString() + ")";
        // }
        // return "Enum(" + variantName + ")";

        if (associatedValue) {
            // Use a generic representation if toString() is not available
            return "Enum(" + variantName + ", <associated value>)";
        }
        return "Enum(" + variantName + ")";
    }
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
                 SumValue,
                 EnumValue,
                 UserDefinedValue>
        data;
    friend std::ostream &operator<<(std::ostream &os, const Value &value);
};

inline ValuePtr EnumValue::create(const std::string& variantName, const TypePtr& enumType, ValuePtr associatedValue) {
    return std::make_shared<Value>(Value{
        enumType,
        EnumValue(variantName, enumType, associatedValue)
    });
}

inline std::ostream &operator<<(std::ostream &os, const Value &value)
{
    os << "Value(" << value.type->toString() << "): ";

    std::visit(overloaded{[&](const std::monostate &) { os << "nil"; },
                          [&](bool b) { os << (b ? "true" : "false"); },
                          [&](int8_t i) { os << static_cast<int>(i); },
                          [&](int16_t i) { os << i; },
                          [&](int32_t i) { os << i; },
                          [&](int64_t i) { os << i; },
                          [&](uint8_t u) { os << static_cast<unsigned>(u); },
                          [&](uint16_t u) { os << u; },
                          [&](uint32_t u) { os << u; },
                          [&](uint64_t u) { os << u; },
                          [&](float f) { os << f; },
                          [&](double d) { os << d; },
                          [&](const std::string &s) { os << s; },
                          [&](const ListValue &lv) {
                              os << "[";
                              for (const auto &elem : lv.elements) {
                                  os << *elem << ", ";
                              }
                              os << "]";
                          },
                          [&](const DictValue &dv) {
                              os << "{";
                              for (const auto &[key, val] : dv.elements) {
                                  os << *key << ": " << *val << ", ";
                              }
                              os << "}";
                          },
                          [&](const SumValue &sv) { os << "Sum(" << sv.activeVariant << ")"; },
                          [&](const UserDefinedValue &uv) {
                              os << "UserDefined(" << uv.variantName << ")";
                          },
                          [&](const auto &) { os << "unknown"; }},
               value.data);

    return os;
}

// Define the operator<< for ListValue
inline std::ostream &operator<<(std::ostream &os, const ListValue &lv)
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

// Define the operator<< for DictValue
inline std::ostream &operator<<(std::ostream &os, const DictValue &dv)
{
    os << "{";
    bool first = true;
    for (const auto &[key, value] : dv.elements) {
        if (!first)
            os << ", ";
        first = false;
        os << key << ": " << value;
    }
    os << "}";
    return os;
}

// Define the operator<< for UserDefinedValue
inline std::ostream &operator<<(std::ostream &os, const UserDefinedValue &udv)
{
    os << udv.variantName << "{";
    bool first = true;
    for (const auto &[field, value] : udv.fields) {
        if (!first)
            os << ", ";
        first = false;
        os << field << ": " << value;
    }
    os << "}";
    return os;
}

// Define the operator<< for SumValue
inline std::ostream &operator<<(std::ostream &os, const SumValue &udv)
{
    os << "Variant" << udv.activeVariant << "(";
    os << *udv.value;
    os << ")";
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const std::monostate &)
{
    return os << "Nil";
}

// Update the operator<< for EnumValue
inline std::ostream &operator<<(std::ostream &os, const EnumValue &ev) {
    os << ev.toString();
    return os;
}
// Define the operator<< for ValuePtr
inline std::ostream &operator<<(std::ostream &os, const ValuePtr &valuePtr)
{
    if (valuePtr) {
        os << *valuePtr;
    } else {
        os << "nullptr";
    }
    return os;
}
