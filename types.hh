//types.hh
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
    std::vector<std::pair<std::string, std::map<std::string, TypePtr>>> fields;
};

// Add a structure to represent sum types
struct SumType
{
    std::vector<TypePtr> variants;
};

struct Type
{
    TypeTag tag;
    std::variant<std::monostate, ListType, DictType, EnumType, FunctionType, SumType, UserDefinedType>
        extra;

    Type(TypeTag t)
        : tag(t)
    {}
    Type(TypeTag t,
         const std::
             variant<std::monostate, ListType, DictType, EnumType, FunctionType, SumType, UserDefinedType>
                 &ex)
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
        case TypeTag::UserDefined:
            return "UserDefined";
        default:
            return "Unknown";
        }
    }

    bool operator==(const Type &other) const { return tag == other.tag; }
    bool operator!=(const Type &other) const { return !(*this == other); }
};

std::string typeTagToString(TypeTag tag)
{
    return Type(tag).toString();
}

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

// Modify the UserDefinedValue to support tagged variants
struct UserDefinedValue
{
    std::string variantName;
    std::map<std::string, ValuePtr> fields;
};

// Add a new variant to the Value struct for sum types
struct SumValue
{
    size_t activeVariant;
    ValuePtr value;
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

template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

class TypeSystem
{
private:
    std::map<std::string, TypePtr> userDefinedTypes;

    bool canConvert(TypePtr from, TypePtr to);
    bool isListType(TypePtr type) const { return type->tag == TypeTag::List; }
    bool isDictType(TypePtr type) const { return type->tag == TypeTag::Dict; }
    ValuePtr stringToNumber(const std::string &str, TypePtr targetType);
    ValuePtr numberToString(const ValuePtr &value);

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

    TypePtr createListType(TypePtr elementType);
    TypePtr createDictType(TypePtr keyType, TypePtr valueType);
    TypePtr createSumType(const std::vector<TypePtr> &variants);
    TypePtr createADT(
        const std::string &name,
        const std::vector<std::pair<std::string, std::map<std::string, TypePtr>>> &variants);
    TypePtr createFunctionType(const std::vector<TypePtr> &paramTypes, TypePtr returnType);

    bool checkListType(const ValuePtr &value, TypePtr elementType);
    bool checkDictType(const ValuePtr &value, TypePtr keyType, TypePtr valueType);
    bool checkSumType(const ValuePtr &value, const std::vector<TypePtr> &variants);
    bool checkADT(const ValuePtr &value, const UserDefinedType &adtType);
    bool checkFunctionType(const ValuePtr &value,
                           const std::vector<TypePtr> &paramTypes,
                           TypePtr returnType);
    TypeTag stringToType(const std::string &typeStr);
    std::string typeToString(const TypePtr &type) const;

    struct TypeMapping
    {
        const char *str;
        TypeTag tag;
    };

    static constexpr std::array<TypeMapping, 23> typeMappings = {
        {{"int", TypeTag::Int},     {"i8", TypeTag::Int8},       {"i16", TypeTag::Int16},
         {"i32", TypeTag::Int32},   {"i64", TypeTag::Int64},     {"i128", TypeTag::Int64},
         {"uint", TypeTag::UInt},   {"u8", TypeTag::UInt8},      {"u16", TypeTag::UInt16},
         {"u32", TypeTag::UInt32},  {"u64", TypeTag::UInt64},    {"u128", TypeTag::UInt64},
         {"f32", TypeTag::Float32}, {"f64", TypeTag::Float64},   {"float", TypeTag::Float64},
         {"bool", TypeTag::Bool},   {"string", TypeTag::String}, {"dict", TypeTag::Dict},
         {"list", TypeTag::List},   {"enum", TypeTag::Enum},     {"any", TypeTag::Any}}};
};

bool TypeSystem::canConvert(TypePtr from, TypePtr to)
{
    if (from == to || to->tag == TypeTag::Any)
        return true;

    if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
         || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64 || from->tag == TypeTag::UInt
         || from->tag == TypeTag::UInt8 || from->tag == TypeTag::UInt16
         || from->tag == TypeTag::UInt32 || from->tag == TypeTag::UInt64)
        && (to->tag == TypeTag::Int || to->tag == TypeTag::Int8 || to->tag == TypeTag::Int16
            || to->tag == TypeTag::Int32 || to->tag == TypeTag::Int64 || to->tag == TypeTag::UInt
            || to->tag == TypeTag::UInt8 || to->tag == TypeTag::UInt16 || to->tag == TypeTag::UInt32
            || to->tag == TypeTag::UInt64))
        return true;

    return false;
}

TypePtr TypeSystem::getUserDefinedType(const std::string &name)
{
    auto it = userDefinedTypes.find(name);
    return it != userDefinedTypes.end() ? it->second : nullptr;
}

void TypeSystem::registerUserDefinedType(const std::string &name, TypePtr type)
{
    userDefinedTypes[name] = type;
}

TypePtr TypeSystem::createListType(TypePtr elementType)
{
    return std::make_shared<Type>(TypeTag::List, ListType{elementType});
}

TypePtr TypeSystem::createDictType(TypePtr keyType, TypePtr valueType)
{
    return std::make_shared<Type>(TypeTag::Dict, DictType{keyType, valueType});
}

inline TypePtr TypeSystem::createSumType(const std::vector<TypePtr> &variants)
{
    return std::make_shared<Type>(TypeTag::Sum, SumType{variants});
}

inline TypePtr TypeSystem::createADT(
    const std::string &name,
    const std::vector<std::pair<std::string, std::map<std::string, TypePtr>>> &variants)
{
    return std::make_shared<Type>(TypeTag::UserDefined, UserDefinedType{name, variants});
}

inline TypePtr TypeSystem::createFunctionType(const std::vector<TypePtr> &paramTypes,
                                              TypePtr returnType)
{
    return std::make_shared<Type>(TypeTag::Function, FunctionType{paramTypes, returnType});
}

bool TypeSystem::checkListType(const ValuePtr &value, TypePtr elementType)
{
    if (!isListType(value->type))
        return false;
    const auto &listValue = std::get<ListValue>(value->data);
    return std::all_of(listValue.elements.begin(),
                       listValue.elements.end(),
                       [&](const ValuePtr &elem) { return checkType(elem, elementType); });
}

bool TypeSystem::checkDictType(const ValuePtr &value, TypePtr keyType, TypePtr valueType)
{
    if (!isDictType(value->type))
        return false;
    const auto &dictValue = std::get<DictValue>(value->data);
    return std::all_of(dictValue.elements.begin(), dictValue.elements.end(), [&](const auto &kv) {
        return checkType(kv.first, keyType) && checkType(kv.second, valueType);
    });
}

inline bool TypeSystem::checkSumType(const ValuePtr &value, const std::vector<TypePtr> &variants)
{
    if (value->type->tag != TypeTag::Sum)
        return false;
    const auto &sumValue = std::get<SumValue>(value->data);
    return sumValue.activeVariant < variants.size()
           && checkType(sumValue.value, variants[sumValue.activeVariant]);
}

inline bool TypeSystem::checkADT(const ValuePtr &value, const UserDefinedType &adtType)
{
    if (value->type->tag != TypeTag::UserDefined)
        return false;
    const auto &udValue = std::get<UserDefinedValue>(value->data);

    auto variantIt = std::find_if(adtType.fields.begin(),
                                  adtType.fields.end(),
                                  [&](const auto &variant) {
                                      return variant.first == udValue.variantName;
                                  });

    if (variantIt == adtType.fields.end())
        return false;

    const auto &expectedFields = variantIt->second;
    for (const auto &[fieldName, fieldType] : expectedFields) {
        auto it = udValue.fields.find(fieldName);
        if (it == udValue.fields.end() || !checkType(it->second, fieldType)) {
            return false;
        }
    }
    return true;
}

inline bool TypeSystem::checkFunctionType(const ValuePtr &value,
                                          const std::vector<TypePtr> &paramTypes,
                                          TypePtr returnType)
{
    if (value->type->tag != TypeTag::Function)
        return false;
    const auto &funcType = std::get<FunctionType>(value->type->extra);
    return funcType.paramTypes == paramTypes && funcType.returnType == returnType;
}

TypePtr TypeSystem::inferType(const ValuePtr &value)
{
    return value->type;
}

bool TypeSystem::checkType(const ValuePtr &value, TypePtr expectedType)
{
    if (expectedType->tag == TypeTag::Any)
        return true;
    if (value->type == expectedType)
        return true;

    switch (expectedType->tag) {
    case TypeTag::List: {
        const auto &expectedListType = std::get<ListType>(expectedType->extra);
        return checkListType(value, expectedListType.elementType);
    }
    case TypeTag::Dict: {
        const auto &expectedDictType = std::get<DictType>(expectedType->extra);
        return checkDictType(value, expectedDictType.keyType, expectedDictType.valueType);
    }
    case TypeTag::Sum: {
        const auto &expectedSumType = std::get<SumType>(expectedType->extra);
        return checkSumType(value, expectedSumType.variants);
    }
    case TypeTag::UserDefined: {
        const auto &expectedUserType = std::get<UserDefinedType>(expectedType->extra);
        return checkADT(value, expectedUserType);
    }
    default:
        // For primitive types, we've already checked equality above
        return false;
    }
}

ValuePtr TypeSystem::convert(const ValuePtr &value, TypePtr targetType)
{
    if (!canConvert(value->type, targetType)) {
        throw std::runtime_error("Cannot convert type " + value->type->toString() + " to "
                                 + targetType->toString());
    }

    ValuePtr result = std::make_shared<Value>();
    result->type = targetType;
    std::visit(overloaded{[&](auto arg) { result->data = arg; },
                          [&](int8_t arg) { result->data = safe_cast<int64_t>(arg); },
                          [&](int16_t arg) { result->data = safe_cast<int64_t>(arg); },
                          [&](int32_t arg) { result->data = safe_cast<int64_t>(arg); },
                          [&](int64_t arg) { result->data = arg; },
                          [&](uint8_t arg) { result->data = safe_cast<uint64_t>(arg); },
                          [&](uint16_t arg) { result->data = safe_cast<uint64_t>(arg); },
                          [&](uint32_t arg) { result->data = safe_cast<uint64_t>(arg); },
                          [&](uint64_t arg) { result->data = arg; },
                          [&](float arg) { result->data = static_cast<double>(arg); },
                          [&](double arg) { result->data = arg; }},
               value->data);

    return result;
}

TypeTag TypeSystem::stringToType(const std::string &typeStr)
{
    auto it = std::find_if(typeMappings.begin(), typeMappings.end(), [&](const TypeMapping &tm) {
        return tm.str == typeStr;
    });
    if (it != typeMappings.end()) {
        return it->tag;
    }
    throw std::invalid_argument("Unknown type string: " + typeStr);
}

ValuePtr TypeSystem::stringToNumber(const std::string &str, TypePtr targetType)
{
    ValuePtr result = std::make_shared<Value>();
    result->type = targetType;

    try {
        if (targetType->tag == TypeTag::Int) {
            result->data = std::stoll(str);
        } else if (targetType->tag == TypeTag::Float32) {
            result->data = std::stof(str);
        } else if (targetType->tag == TypeTag::Float64) {
            result->data = std::stod(str);
        }
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to convert string to number: " + std::string(e.what()));
    }

    return result;
}

ValuePtr TypeSystem::numberToString(const ValuePtr &value)
{
    ValuePtr result = std::make_shared<Value>();
    result->type = STRING_TYPE;

    std::visit(overloaded{[&](int64_t v) { result->data = std::to_string(v); },
                          [&](double v) { result->data = std::to_string(v); },
                          [&](auto) {
                              throw std::runtime_error("Unexpected type in numberToString");
                          }},
               value->data);

    return result;
}

std::string TypeSystem::typeToString(const TypePtr &type) const
{
    std::string result = type->toString();

    if (type->tag == TypeTag::List) {
        const auto &listType = std::get<ListType>(type->extra);
        result += "<" + typeToString(listType.elementType) + ">";
    } else if (type->tag == TypeTag::Dict) {
        const auto &dictType = std::get<DictType>(type->extra);
        result += "<" + typeToString(dictType.keyType) + ", " + typeToString(dictType.valueType)
                  + ">";
    } else if (type->tag == TypeTag::UserDefined) {
        const auto &userType = std::get<UserDefinedType>(type->extra);
        result += " " + userType.name;
    }

    return result;
}

// Modify the existing operator<< for Value to use the new typeToString function
std::ostream &operator<<(std::ostream &os, const Value &value)
{
    os << value.type->toString() << "(";
    std::visit(overloaded{[&](std::monostate) { os << "Nil"; },
                          [&](bool v) { os << (v ? "true" : "false"); },
                          [&](int8_t v) { os << static_cast<int32_t>(v); },
                          [&](int16_t v) { os << v; },
                          [&](int32_t v) { os << v; },
                          [&](int64_t v) { os << v; },
                          [&](uint8_t v) { os << static_cast<uint32_t>(v); },
                          [&](uint16_t v) { os << v; },
                          [&](uint32_t v) { os << v; },
                          [&](uint64_t v) { os << v; },
                          [&](float v) { os << v; },
                          [&](double v) { os << v; },
                          [&](const std::string &v) { os << '"' << v << '"'; },
                          [&](const ListValue &v) {
                              os << "[";
                              for (size_t i = 0; i < v.elements.size(); ++i) {
                                  os << *v.elements[i];
                                  if (i < v.elements.size() - 1)
                                      os << ", ";
                              }
                              os << "]";
                          },
                          [&](const DictValue &v) {
                              os << "{";
                              size_t i = 0;
                              for (const auto &[key, val] : v.elements) {
                                  os << *key << ": " << *val;
                                  if (++i < v.elements.size())
                                      os << ", ";
                              }
                              os << "}";
                          },
                          [&](const UserDefinedValue &v) {
                              os << "{";
                              size_t i = 0;
                              for (const auto &[field, val] : v.fields) {
                                  os << field << ": " << *val;
                                  if (++i < v.fields.size())
                                      os << ", ";
                              }
                              os << "}";
                          }},
               value.data);
    os << ")";
    return os;
}
// Add an operator<< for Type

std::ostream &operator<<(std::ostream &os, const Type &type)
{
    os << TypeSystem().typeToString(std::make_shared<Type>(type));
    return os;
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
