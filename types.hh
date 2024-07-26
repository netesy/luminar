//types.hh
#pragma once

//#include <algorithm>
//#include <cstdint>
//#include <iostream>
//#include <map>
//#include <memory>
//#include <stdexcept>
//#include <string>
//#include <variant>
//#include <vector>

//enum class TypeTag {
//    Nil,
//    Bool,
//    Int,
//    Int8,
//    Int16,
//    Int32,
//    Int64,
//    UInt,
//    UInt8,
//    UInt16,
//    UInt32,
//    UInt64,
//    Float32,
//    Float64,
//    String,
//    List,
//    Dict,
//    Enum,
//    Function,
//    Any,
//    UserDefined
//};

//struct Type;
//using TypePtr = std::shared_ptr<Type>;

//struct ListType
//{
//    TypePtr elementType;
//};

//struct DictType
//{
//    TypePtr keyType;
//    TypePtr valueType;
//};

//struct EnumType
//{
//    std::vector<std::string> values;
//};

//struct FunctionType
//{
//    std::vector<TypePtr> paramTypes;
//    TypePtr returnType;
//};

//struct UserDefinedType
//{
//    std::string name;
//    std::map<std::string, TypePtr> fields;
//};

//struct Type
//{
//    TypeTag tag;
//    std::variant<std::monostate, ListType, DictType, EnumType, FunctionType, UserDefinedType> extra;

//    Type(TypeTag t)
//        : tag(t)
//    {}
//    Type(TypeTag t, ListType lt)
//        : tag(t)
//        , extra(lt)
//    {}
//    Type(TypeTag t, DictType dt)
//        : tag(t)
//        , extra(dt)
//    {}
//    Type(TypeTag t, EnumType et)
//        : tag(t)
//        , extra(et)
//    {}
//    Type(TypeTag t, FunctionType ft)
//        : tag(t)
//        , extra(ft)
//    {}
//    Type(TypeTag t, UserDefinedType udt)
//        : tag(t)
//        , extra(udt)
//    {}

//    std::string toString() const
//    {
//        switch (tag) {
//        case TypeTag::Nil:
//            return "Nil";
//        case TypeTag::Bool:
//            return "Bool";
//        case TypeTag::Int:
//            return "Int";
//        case TypeTag::Int8:
//            return "Int8";
//        case TypeTag::Int16:
//            return "Int16";
//        case TypeTag::Int32:
//            return "Int32";
//        case TypeTag::Int64:
//            return "Int64";
//        case TypeTag::UInt:
//            return "UInt";
//        case TypeTag::UInt8:
//            return "UInt8";
//        case TypeTag::UInt16:
//            return "UInt16";
//        case TypeTag::UInt32:
//            return "UInt32";
//        case TypeTag::UInt64:
//            return "UInt64";
//        case TypeTag::Float32:
//            return "Float32";
//        case TypeTag::Float64:
//            return "Float64";
//        case TypeTag::String:
//            return "String";
//        case TypeTag::List:
//            return "List";
//        case TypeTag::Dict:
//            return "Dict";
//        case TypeTag::Enum:
//            return "Enum";
//        case TypeTag::Function:
//            return "Function";
//        case TypeTag::Any:
//            return "Any";
//        case TypeTag::UserDefined:
//            return "UserDefined";
//        default:
//            return "Unknown";
//        }
//    }

//    bool operator==(const Type &other) const { return tag == other.tag; }

//    bool operator!=(const Type &other) const { return !(*this == other); }
//};

//std::string typeTagToString(TypeTag tag)
//{
//    return Type(tag).toString();
//}

//constexpr int getSizeInBits(TypeTag tag)
//{
//    switch (tag) {
//    case TypeTag::Int8:
//    case TypeTag::UInt8:
//        return 8;
//    case TypeTag::Int16:
//    case TypeTag::UInt16:
//        return 16;
//    case TypeTag::Int:
//    case TypeTag::UInt:
//    case TypeTag::Int32:
//    case TypeTag::UInt32:
//    case TypeTag::Float32:
//        return 32;
//    case TypeTag::Int64:
//    case TypeTag::UInt64:
//    case TypeTag::Float64:
//        return 64;
//    default:
//        return 0;
//    }
//}

//struct Value;
//using ValuePtr = std::shared_ptr<Value>;

//struct ListValue
//{
//    std::vector<ValuePtr> elements;
//};

//struct DictValue
//{
//    std::map<ValuePtr, ValuePtr> elements;
//};

//struct UserDefinedValue
//{
//    std::map<std::string, ValuePtr> fields;
//};

//struct Value
//{
//    TypePtr type;
//    std::variant<std::monostate,
//                 bool,
//                 int8_t,
//                 int16_t,
//                 int32_t,
//                 int64_t,
//                 uint8_t,
//                 uint16_t,
//                 uint32_t,
//                 uint64_t,
//                 double,
//                 float,
//                 std::string,
//                 ListValue,
//                 DictValue,
//                 UserDefinedValue>
//        data;
//    friend std::ostream &operator<<(std::ostream &os, const Value &value);
//};

//class OverflowException : public std::runtime_error
//{
//public:
//    OverflowException(const std::string &msg)
//        : std::runtime_error(msg)
//    {}
//};

//template<typename To, typename From>
//To safe_cast(From value)
//{
//    To result = static_cast<To>(value);
//    if (static_cast<From>(result) != value || (value > 0 && result < 0)
//        || (value < 0 && result > 0)) {
//        throw OverflowException("Overflow detected in integer conversion");
//    }
//    return result;
//}

//// Define the overloaded utility
//template<class... Ts>
//struct overloaded : Ts...
//{
//    using Ts::operator()...;
//};
//// Explicit deduction guide (not needed as of C++20)
//template<class... Ts>
//overloaded(Ts...) -> overloaded<Ts...>;

//class TypeSystem
//{
//private:
//    std::map<std::string, TypePtr> userDefinedTypes;

//    bool canConvert(TypePtr from, TypePtr to);
//    bool isListType(TypePtr type) const { return type->tag == TypeTag::List; }
//    bool isDictType(TypePtr type) const { return type->tag == TypeTag::Dict; }

//public:
//    const TypePtr NIL_TYPE = std::make_shared<Type>(TypeTag::Nil);
//    const TypePtr BOOL_TYPE = std::make_shared<Type>(TypeTag::Bool);
//    const TypePtr INT_TYPE = std::make_shared<Type>(TypeTag::Int);
//    const TypePtr INT8_TYPE = std::make_shared<Type>(TypeTag::Int8);
//    const TypePtr INT16_TYPE = std::make_shared<Type>(TypeTag::Int16);
//    const TypePtr INT32_TYPE = std::make_shared<Type>(TypeTag::Int32);
//    const TypePtr INT64_TYPE = std::make_shared<Type>(TypeTag::Int64);
//    const TypePtr UINT_TYPE = std::make_shared<Type>(TypeTag::UInt);
//    const TypePtr UINT8_TYPE = std::make_shared<Type>(TypeTag::UInt8);
//    const TypePtr UINT16_TYPE = std::make_shared<Type>(TypeTag::UInt16);
//    const TypePtr UINT32_TYPE = std::make_shared<Type>(TypeTag::UInt32);
//    const TypePtr UINT64_TYPE = std::make_shared<Type>(TypeTag::UInt64);
//    const TypePtr FLOAT32_TYPE = std::make_shared<Type>(TypeTag::Float32);
//    const TypePtr FLOAT64_TYPE = std::make_shared<Type>(TypeTag::Float64);
//    const TypePtr STRING_TYPE = std::make_shared<Type>(TypeTag::String);
//    const TypePtr ANY_TYPE = std::make_shared<Type>(TypeTag::Any);

//    TypePtr getUserDefinedType(const std::string &name);
//    void registerUserDefinedType(const std::string &name, TypePtr type);
//    ValuePtr convert(const ValuePtr &value, TypePtr targetType);
//    TypePtr inferType(const ValuePtr &value);
//    bool checkType(const ValuePtr &value, TypePtr expectedType);

//    // New methods for ListType and DictType
//    TypePtr createListType(TypePtr elementType);
//    TypePtr createDictType(TypePtr keyType, TypePtr valueType);
//    bool checkListType(const ValuePtr &value, TypePtr elementType);
//    bool checkDictType(const ValuePtr &value, TypePtr keyType, TypePtr valueType);
//    TypeTag stringToType(const std::string &typeStr);

//    struct TypeMapping
//    {
//        const char *str;
//        TypeTag tag;
//    };

//    // Array of type mappings
//    static constexpr std::array<TypeMapping, 23> typeMappings = {
//        {{"int", TypeTag::Int},     {"i8", TypeTag::Int8},       {"i16", TypeTag::Int16},
//         {"i32", TypeTag::Int32},   {"i64", TypeTag::Int64},     {"i128", TypeTag::Int64},
//         {"uint", TypeTag::UInt},   {"u8", TypeTag::UInt8},      {"u16", TypeTag::UInt16},
//         {"u32", TypeTag::UInt32},  {"u64", TypeTag::UInt64},    {"u128", TypeTag::UInt64},
//         {"f32", TypeTag::Float32}, {"f64", TypeTag::Float64},   {"float", TypeTag::Float64},
//         {"bool", TypeTag::Bool},   {"string", TypeTag::String}, {"dict", TypeTag::Dict},
//         {"list", TypeTag::List},   {"enum", TypeTag::Enum},     {"any", TypeTag::Any}}};
//};

//bool TypeSystem::canConvert(TypePtr from, TypePtr to)
//{
//    if (from == to)
//        return true;
//    if (to->tag == TypeTag::Any)
//        return true;

//    if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
//         || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64 || from->tag == TypeTag::UInt
//         || from->tag == TypeTag::UInt8 || from->tag == TypeTag::UInt16
//         || from->tag == TypeTag::UInt32 || from->tag == TypeTag::UInt64)
//        && (to->tag == TypeTag::Int || to->tag == TypeTag::Int8 || to->tag == TypeTag::Int16
//            || to->tag == TypeTag::Int32 || to->tag == TypeTag::Int64 || to->tag == TypeTag::UInt
//            || to->tag == TypeTag::UInt8 || to->tag == TypeTag::UInt16 || to->tag == TypeTag::UInt32
//            || to->tag == TypeTag::UInt64)) {
//        if (getSizeInBits(from->tag) > getSizeInBits(to->tag)) {
//            std::cout << "Warning: Potential data loss in conversion from "
//                      << typeTagToString(from->tag) << " to " << typeTagToString(to->tag)
//                      << std::endl;
//        }
//        return true;
//    }

//    if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
//         || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64 || from->tag == TypeTag::UInt
//         || from->tag == TypeTag::UInt8 || from->tag == TypeTag::UInt16
//         || from->tag == TypeTag::UInt32 || from->tag == TypeTag::UInt64)
//        && (to->tag == TypeTag::Float32 || to->tag == TypeTag::Float64))
//        return true;

//    if ((from->tag == TypeTag::Int || from->tag == TypeTag::Int8 || from->tag == TypeTag::Int16
//         || from->tag == TypeTag::Int32 || from->tag == TypeTag::Int64 || from->tag == TypeTag::UInt
//         || from->tag == TypeTag::UInt8 || from->tag == TypeTag::UInt16
//         || from->tag == TypeTag::UInt32 || from->tag == TypeTag::UInt64)
//        && to->tag == TypeTag::String)
//        return true;

//    if ((from->tag == TypeTag::Float32 || from->tag == TypeTag::Float64) && to->tag == TypeTag::Int)
//        return true;

//    if ((from->tag == TypeTag::Float32 || from->tag == TypeTag::Float64)
//        && to->tag == TypeTag::String)
//        return true;

//    return false;
//}

//TypePtr TypeSystem::getUserDefinedType(const std::string &name)
//{
//    auto it = userDefinedTypes.find(name);
//    if (it != userDefinedTypes.end())
//        return it->second;
//    else
//        return nullptr;
//}

//void TypeSystem::registerUserDefinedType(const std::string &name, TypePtr type)
//{
//    userDefinedTypes[name] = type;
//}

//ValuePtr TypeSystem::convert(const ValuePtr &value, TypePtr targetType)
//{
//    if (value->type == targetType) {
//        return value;
//    }

//    if (!canConvert(value->type, targetType)) {
//        throw std::invalid_argument("Unsupported conversion");
//    }

//    ValuePtr newValue = std::make_shared<Value>();
//    newValue->type = targetType;

//    // Helper lambda for numeric conversions
//    auto numericConversion = [&](auto sourceValue) {
//        using T = decltype(sourceValue);
//        if (targetType == INT8_TYPE)
//            newValue->data = safe_cast<int8_t>(sourceValue);
//        else if (targetType == INT16_TYPE)
//            newValue->data = safe_cast<int16_t>(sourceValue);
//        else if (targetType == INT32_TYPE)
//            newValue->data = safe_cast<int32_t>(sourceValue);
//        else if (targetType == INT64_TYPE)
//            newValue->data = static_cast<int64_t>(sourceValue);
//        else if (targetType == UINT_TYPE)
//            newValue->data = safe_cast<unsigned int>(sourceValue);
//        else if (targetType == UINT8_TYPE)
//            newValue->data = safe_cast<uint8_t>(sourceValue);
//        else if (targetType == UINT16_TYPE)
//            newValue->data = safe_cast<uint16_t>(sourceValue);
//        else if (targetType == UINT32_TYPE)
//            newValue->data = safe_cast<uint32_t>(sourceValue);
//        else if (targetType == UINT64_TYPE)
//            newValue->data = safe_cast<uint64_t>(sourceValue);
//        else if (targetType == FLOAT32_TYPE)
//            newValue->data = static_cast<float>(sourceValue);
//        else if (targetType == FLOAT64_TYPE)
//            newValue->data = static_cast<double>(sourceValue);
//        else if (targetType == STRING_TYPE)
//            newValue->data = std::to_string(sourceValue);
//        else
//            throw std::invalid_argument("Unsupported conversion");
//    };

//    // Main conversion logic
//    std::visit(
//        overloaded{[&](int intValue) { numericConversion(intValue); },
//                   [&](float floatValue) {
//                       if (targetType == INT_TYPE)
//                           newValue->data = static_cast<int>(floatValue);
//                       else if (targetType == STRING_TYPE)
//                           newValue->data = std::to_string(floatValue);
//                       else
//                           numericConversion(floatValue);
//                   },
//                   [&](double doubleValue) {
//                       if (targetType == INT_TYPE)
//                           newValue->data = static_cast<int>(doubleValue);
//                       else if (targetType == STRING_TYPE)
//                           newValue->data = std::to_string(doubleValue);
//                       else
//                           numericConversion(doubleValue);
//                   },
//                   [&](const std::string &stringValue) {
//                       if (targetType == INT_TYPE)
//                           newValue->data = std::stoi(stringValue);
//                       else if (targetType == FLOAT32_TYPE)
//                           newValue->data = std::stof(stringValue);
//                       else if (targetType == FLOAT64_TYPE)
//                           newValue->data = std::stod(stringValue);
//                       else
//                           throw std::invalid_argument("Unsupported conversion");
//                   },
//                   [&](const ListValue &listValue) {
//                       if (isListType(targetType)) {
//                           const auto &sourceListType = std::get<ListType>(value->type->extra);
//                           const auto &targetListType = std::get<ListType>(targetType->extra);
//                           if (canConvert(sourceListType.elementType, targetListType.elementType)) {
//                               auto &newElements = newValue->data.emplace<ListValue>().elements;
//                               for (const auto &element : listValue.elements) {
//                                   newElements.push_back(
//                                       convert(element, targetListType.elementType));
//                               }
//                               return;
//                           }
//                       }
//                       throw std::invalid_argument("Unsupported list conversion");
//                   },
//                   [&](const DictValue &dictValue) {
//                       if (isDictType(targetType)) {
//                           const auto &sourceDictType = std::get<DictType>(value->type->extra);
//                           const auto &targetDictType = std::get<DictType>(targetType->extra);
//                           if (canConvert(sourceDictType.keyType, targetDictType.keyType)
//                               && canConvert(sourceDictType.valueType, targetDictType.valueType)) {
//                               auto &newElements = newValue->data.emplace<DictValue>().elements;
//                               for (const auto &[key, val] : dictValue.elements) {
//                                   auto newKey = convert(key, targetDictType.keyType);
//                                   auto newVal = convert(val, targetDictType.valueType);
//                                   newElements[newKey] = newVal;
//                               }
//                               return;
//                           }
//                       }
//                       throw std::invalid_argument("Unsupported dict conversion");
//                   },
//                   [](auto) { throw std::invalid_argument("Unsupported conversion"); }},
//        value->data);

//    return newValue;
//}

//TypePtr TypeSystem::inferType(const ValuePtr &value)
//{
//    if (isListType(value->type)) {
//        const auto &listValue = std::get<ListValue>(value->data);
//        if (!listValue.elements.empty()) {
//            auto elementType = inferType(listValue.elements[0]);
//            for (size_t i = 1; i < listValue.elements.size(); ++i) {
//                auto currentType = inferType(listValue.elements[i]);
//                if (currentType != elementType) {
//                    elementType = ANY_TYPE;
//                    break;
//                }
//            }
//            return createListType(elementType);
//        }
//        return createListType(ANY_TYPE);
//    }

//    if (isDictType(value->type)) {
//        const auto &dictValue = std::get<DictValue>(value->data);
//        if (!dictValue.elements.empty()) {
//            auto it = dictValue.elements.begin();
//            auto keyType = inferType(it->first);
//            auto valueType = inferType(it->second);
//            for (++it; it != dictValue.elements.end(); ++it) {
//                auto currentKeyType = inferType(it->first);
//                auto currentValueType = inferType(it->second);
//                if (currentKeyType != keyType) {
//                    keyType = ANY_TYPE;
//                }
//                if (currentValueType != valueType) {
//                    valueType = ANY_TYPE;
//                }
//                if (keyType == ANY_TYPE && valueType == ANY_TYPE) {
//                    break;
//                }
//            }
//            return createDictType(keyType, valueType);
//        }
//        return createDictType(ANY_TYPE, ANY_TYPE);
//    }

//    return value->type;
//}

//bool TypeSystem::checkType(const ValuePtr &value, TypePtr expectedType)
//{
//    if (value->type == expectedType) {
//        return true;
//    }

//    if (isListType(value->type) && isListType(expectedType)) {
//        const auto &expectedListType = std::get<ListType>(expectedType->extra);
//        return checkListType(value, expectedListType.elementType);
//    }

//    if (isDictType(value->type) && isDictType(expectedType)) {
//        const auto &expectedDictType = std::get<DictType>(expectedType->extra);
//        return checkDictType(value, expectedDictType.keyType, expectedDictType.valueType);
//    }

//    return canConvert(value->type, expectedType);
//}

//TypePtr TypeSystem::createListType(TypePtr elementType)
//{
//    auto listType = std::make_shared<Type>(TypeTag::List);
//    listType->extra = ListType{elementType};
//    return listType;
//}

//TypePtr TypeSystem::createDictType(TypePtr keyType, TypePtr valueType)
//{
//    auto dictType = std::make_shared<Type>(TypeTag::Dict);
//    dictType->extra = DictType{keyType, valueType};
//    return dictType;
//}

//bool TypeSystem::checkListType(const ValuePtr &value, TypePtr elementType)
//{
//    if (!isListType(value->type)) {
//        return false;
//    }

//    const auto &listValue = std::get<ListValue>(value->data);
//    for (const auto &element : listValue.elements) {
//        if (!checkType(element, elementType)) {
//            return false;
//        }
//    }
//    return true;
//}

//bool TypeSystem::checkDictType(const ValuePtr &value, TypePtr keyType, TypePtr valueType)
//{
//    if (!isDictType(value->type)) {
//        return false;
//    }

//    const auto &dictValue = std::get<DictValue>(value->data);
//    for (const auto &[key, val] : dictValue.elements) {
//        if (!checkType(key, keyType) || !checkType(val, valueType)) {
//            return false;
//        }
//    }
//    return true;
//}

//inline TypeTag TypeSystem::stringToType(const std::string &typeStr)
//{
//    auto it = std::find_if(typeMappings.begin(),
//                           typeMappings.end(),
//                           [&typeStr](const TypeMapping &mapping) {
//                               return typeStr == mapping.str;
//                           });

//    if (it != typeMappings.end()) {
//        return it->tag;
//    }

//    return TypeTag::UserDefined;
//}

//// Custom operator<< for std::monostate
//std::ostream &operator<<(std::ostream &os, const std::monostate &)
//{
//    return os << "monostate";
//}

//// Custom operator<< for ListValue
//std::ostream &operator<<(std::ostream &os, const ListValue &lv)
//{
//    os << "[";
//    for (size_t i = 0; i < lv.elements.size(); ++i) {
//        if (i > 0)
//            os << ", ";
//        os << lv.elements[i];
//    }
//    os << "]";
//    return os;
//}

//// Custom operator<< for DictValue
//std::ostream &operator<<(std::ostream &os, const DictValue &dv)
//{
//    os << "{";
//    bool first = true;
//    for (const auto &kv : dv.elements) {
//        if (!first)
//            os << ", ";
//        first = false;
//        os << kv.first << ": " << kv.second;
//    }
//    os << "}";
//    return os;
//}
//std::ostream &operator<<(std::ostream &os, const Value &value)
//{
//    // Implement the output logic for Value
//    // Example: os << value.someMember;
//    return os;
//}

//// Custom operator<< for UserDefinedValue
//std::ostream &operator<<(std::ostream &os, const UserDefinedValue &udv)
//{
//    os << "{";
//    const auto &fields = udv.fields;
//    bool first = true;
//    for (const auto &field : fields) {
//        if (!first)
//            os << ", ";
//        first = false;
//        os << field.first << ": " << *(field.second);
//    }
//    os << "}";
//    return os;
//}

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
    Type(TypeTag t,
         const std::variant<std::monostate, ListType, DictType, EnumType, FunctionType, UserDefinedType>
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
    bool checkListType(const ValuePtr &value, TypePtr elementType);
    bool checkDictType(const ValuePtr &value, TypePtr keyType, TypePtr valueType);
    TypeTag stringToType(const std::string &typeStr);

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
    if (expectedType->tag == TypeTag::List) {
        const auto &expectedListType = std::get<ListType>(expectedType->extra);
        return checkListType(value, expectedListType.elementType);
    }
    if (expectedType->tag == TypeTag::Dict) {
        const auto &expectedDictType = std::get<DictType>(expectedType->extra);
        return checkDictType(value, expectedDictType.keyType, expectedDictType.valueType);
    }
    return false;
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

std::ostream &operator<<(std::ostream &os, const Value &value)
{
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
                          [&](const std::string &v) { os << v; },
                          [&](const ListValue &v) {
                              os << "[";
                              for (const auto &elem : v.elements) {
                                  os << *elem << ", ";
                              }
                              os << "]";
                          },
                          [&](const DictValue &v) {
                              os << "{";
                              for (const auto &[key, val] : v.elements) {
                                  os << *key << ": " << *val << ", ";
                              }
                              os << "}";
                          },
                          [&](const UserDefinedValue &v) {
                              os << "{";
                              for (const auto &[field, val] : v.fields) {
                                  os << field << ": " << *val << ", ";
                              }
                              os << "}";
                          }},
               value.data);
    return os;
}
