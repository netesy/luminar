//types.hh
#pragma once

#include "memory.hh"
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
#include "value.hh"

class TypeSystem
{
private:
    std::map<std::string, TypePtr> userDefinedTypes;
    std::map<std::string, TypePtr> typeAliases;
    MemoryManager<> &memoryManager;
    MemoryManager<>::Region &region;

    bool canConvert(TypePtr from, TypePtr to)
    {
        if (from == to || to->tag == TypeTag::Any)
            return true;

    // Handle boolean type conversions
    if (from->tag == TypeTag::Bool && to->tag == TypeTag::Bool) {
        return true;
    }   

        // Numeric type conversions with range checks
        if (isNumericType(from->tag) && isNumericType(to->tag)) {
            // Check if conversion is safe (no overflow/precision loss)
            return isSafeNumericConversion(from->tag, to->tag);
        }

        // Handle list and dict type compatibility
        if (from->tag == TypeTag::List && to->tag == TypeTag::List) {
            auto fromListType = std::get<ListType>(from->extra);
            auto toListType = std::get<ListType>(to->extra);
            return canConvert(fromListType.elementType, toListType.elementType);
        }

        if (from->tag == TypeTag::Dict && to->tag == TypeTag::Dict) {
            auto fromDictType = std::get<DictType>(from->extra);
            auto toDictType = std::get<DictType>(to->extra);
            return canConvert(fromDictType.keyType, toDictType.keyType) &&
                   canConvert(fromDictType.valueType, toDictType.valueType);
        }

        // Union and sum type compatibility
        if (from->tag == TypeTag::Union) {
            auto unionTypes = std::get<UnionType>(from->extra).types;
            return std::any_of(unionTypes.begin(), unionTypes.end(),
                               [&](TypePtr type) { return canConvert(type, to); });
        }

        return false;
    }
    TypePtr getWiderType(TypePtr a, TypePtr b) {
        // Define type hierarchy (from narrowest to widest)
        static const std::unordered_map<TypeTag, int> typeRanks = {
            {TypeTag::Int8, 0},    {TypeTag::UInt8, 1},
            {TypeTag::Int16, 2},   {TypeTag::UInt16, 3},
            {TypeTag::Int32, 4},   {TypeTag::UInt32, 5},
            {TypeTag::Int64, 6},   {TypeTag::UInt64, 7},
            {TypeTag::Float32, 8}, {TypeTag::Float64, 9}
        };

        auto itA = typeRanks.find(a->tag);
        auto itB = typeRanks.find(b->tag);

        if (itA == typeRanks.end() || itB == typeRanks.end()) {
            throw std::runtime_error("Invalid numeric type in type promotion");
        }

        return (itA->second >= itB->second) ? a : b;
    }
    bool isNumericType(TypeTag tag) {
        return tag == TypeTag::Int8 || tag == TypeTag::Int16 || tag == TypeTag::Int32 ||
        tag == TypeTag::Int64 || tag == TypeTag::UInt8 || tag == TypeTag::UInt16 ||
        tag == TypeTag::UInt32 || tag == TypeTag::UInt64 || tag == TypeTag::Float32 ||
        tag == TypeTag::Float64;
    }
    bool isListType(TypePtr type) const { return type->tag == TypeTag::List; }
    bool isDictType(TypePtr type) const { return type->tag == TypeTag::Dict; }
    bool isSafeNumericConversion(TypeTag from, TypeTag to) {
        // Conversion matrix to determine safe numeric conversions
        switch (from) {
        case TypeTag::Int8:
            return to == TypeTag::Int8 ||
                   to == TypeTag::Int16 ||
                   to == TypeTag::Int ||
                   to == TypeTag::Int32 ||
                   to == TypeTag::Int64 ||
                   to == TypeTag::Float32 ||
                   to == TypeTag::Float64;

        case TypeTag::Int16:
            return to == TypeTag::Int16 ||
                   to == TypeTag::Int ||
                   to == TypeTag::Int32 ||
                   to == TypeTag::Int64 ||
                   to == TypeTag::Float32 ||
                   to == TypeTag::Float64;

        case TypeTag::Int32:
            return to == TypeTag::Int32 ||
            to == TypeTag::Int ||
                   to == TypeTag::Int64 ||
                   to == TypeTag::Float32 ||
                   to == TypeTag::Float64;

        case TypeTag::Int64:
            return to == TypeTag::Int64 ||
                   to == TypeTag::Float64;

        case TypeTag::UInt8:
            return to == TypeTag::UInt8 ||
                   to == TypeTag::UInt16 ||
                   to == TypeTag::UInt ||
                   to == TypeTag::UInt32 ||
                   to == TypeTag::UInt64 ||
                   to == TypeTag::Int16 ||
                   to == TypeTag::Int ||
                   to == TypeTag::Int32 ||
                   to == TypeTag::Int64 ||
                   to == TypeTag::Float32 ||
                   to == TypeTag::Float64;

        case TypeTag::UInt16:
            return to == TypeTag::UInt16 ||
                   to == TypeTag::UInt ||
                   to == TypeTag::UInt32 ||
                   to == TypeTag::UInt64 ||
                   to == TypeTag::Int32 ||
                   to == TypeTag::Int64 ||
                   to == TypeTag::Float32 ||
                   to == TypeTag::Float64;

        case TypeTag::UInt32:
            return to == TypeTag::UInt32 ||
                   to == TypeTag::UInt ||
                   to == TypeTag::UInt64 ||
                   to == TypeTag::Int64 ||
                   to == TypeTag::Float64;

        case TypeTag::UInt64:
            return to == TypeTag::UInt64 ||
                   to == TypeTag::Float64;

        case TypeTag::Float32:
            return to == TypeTag::Float32 ||
                   to == TypeTag::Float64;

        case TypeTag::Float64:
            return to == TypeTag::Float64;

        default:
            return false;
        }
    }
    ValuePtr stringToNumber(const std::string &str, TypePtr targetType)
    {
        ValuePtr result = memoryManager.makeRef<Value>(region);
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
    ValuePtr numberToString(const ValuePtr &value)
    {
        ValuePtr result = memoryManager.makeRef<Value>(region);
        result->type = STRING_TYPE;

        std::visit(overloaded{[&](int64_t v) { result->data = std::to_string(v); },
                              [&](double v) { result->data = std::to_string(v); },
                              [&](auto) {
                                  throw std::runtime_error("Unexpected type in numberToString");
                              }},
                   value->data);

        return result;
    }

public:
    TypeSystem(MemoryManager<> &memManager, MemoryManager<>::Region &reg)
        : memoryManager(memManager), region(reg) {}

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
    const TypePtr LIST_TYPE = std::make_shared<Type>(TypeTag::List);
    const TypePtr DICT_TYPE = std::make_shared<Type>(TypeTag::Dict);
    const TypePtr ENUM_TYPE = std::make_shared<Type>(TypeTag::Enum);
    const TypePtr SUM_TYPE = std::make_shared<Type>(TypeTag::Sum);

    ValuePtr createValue(TypePtr type)
    {
        ValuePtr value = memoryManager.makeRef<Value>(region);
        value->type = type;

        switch (type->tag) {
        case TypeTag::Nil:
            value->data = std::monostate{};
            break;
        case TypeTag::Bool:
            value->data = false;
            break;
        case TypeTag::Int64:
            value->data = int64_t(0);
            break;
        case TypeTag::Int8:
            value->data = int8_t(0);
            break;
        case TypeTag::Int16:
            value->data = int16_t(0);
            break;
        case TypeTag::Int:
        case TypeTag::Int32:
            value->data = int32_t(0);
            break;
        case TypeTag::UInt64:
            value->data = uint64_t(0);
            break;
        case TypeTag::UInt8:
            value->data = uint8_t(0);
            break;
        case TypeTag::UInt16:
            value->data = uint16_t(0);
            break;
        case TypeTag::UInt:
        case TypeTag::UInt32:
            value->data = uint32_t(0);
            break;
        case TypeTag::Float32:
            value->data = float(0.0);
            break;
        case TypeTag::Float64:
            value->data = double(0.0);
            break;
        case TypeTag::String:
            value->data = std::string("");
            break;
        case TypeTag::List:
            value->data = ListValue{};
            break;
        case TypeTag::Dict:
            value->data = DictValue{};
            break;
        case TypeTag::Enum:
            // For enums, we'll set it to the first value in the enum
            if (const auto *enumType = std::get_if<EnumType>(&type->extra)) {
                if (!enumType->values.empty()) {
                    value->data = enumType->values[0];
                } else {
                    value->data = std::string(""); // Empty enum, use empty string as default
                }
            } else {
                throw std::runtime_error("Invalid enum type");
            }
            break;
        case TypeTag::Sum:
            // For sum types, we'll set it to the first variant with a default value
            if (const auto *sumType = std::get_if<SumType>(&type->extra)) {
                if (!sumType->variants.empty()) {
                    value->data = SumValue{0, createValue(sumType->variants[0])};
                } else {
                    throw std::runtime_error("Empty sum type");
                }
            } else {
                throw std::runtime_error("Invalid sum type");
            }
            break;

        case TypeTag::Union:
            // For union types, we'll set it to the first type with a default value
            if (const auto *unionType = std::get_if<UnionType>(&type->extra)) {
                if (!unionType->types.empty()) {
                    value->data = createValue(unionType->types[0])->data;
                } else {
                    throw std::runtime_error("Empty union type");
                }
            } else {
                throw std::runtime_error("Invalid union type");
            }
            break;
        case TypeTag::UserDefined:
            value->data = UserDefinedValue{};
            break;
        case TypeTag::Function:
            // Functions are typically not instantiated as values
            throw std::runtime_error("Cannot create a value for Function type");
        case TypeTag::Any:
            // For Any type, we'll use std::monostate as a placeholder
            value->data = std::monostate{};
            break;
        default:
            throw std::runtime_error("Unsupported type tag: "
                                     + std::to_string(static_cast<int>(type->tag)));
        }

        return value;
    }

    bool isCompatible(TypePtr source, TypePtr target) { return canConvert(source, target); }

    TypePtr getCommonType(TypePtr a, TypePtr b)
    {   if(!a || !b)
            return nullptr;
    // Handle Any type
    if (a->tag == TypeTag::Any) return b;
    if (b->tag == TypeTag::Any) return a;

    // Handle Nil type
    if (a->tag == TypeTag::Nil) return b;
    if (b->tag == TypeTag::Nil) return a;

    // Handle Bool type
    if (a->tag == TypeTag::Bool && b->tag == TypeTag::Bool) {
        return a;
    }

    // If types are the same, return either
    if (a->tag == b->tag) {
        return a;
    }

    // Handle numeric type promotions
    if (isNumericType(a->tag) && isNumericType(b->tag)) {
        return getWiderType(a, b);
    }

    // Handle other type conversions
    if (canConvert(a, b)) return b;
    if (canConvert(b, a)) return a;
    throw std::runtime_error("Incompatible types: " + a->toString() + " and " + b->toString());
    }

    void addUserDefinedType(const std::string &name, TypePtr type)
    {
        userDefinedTypes[name] = type;
    }

    TypePtr getUserDefinedType(const std::string &name)
    {
        if (userDefinedTypes.find(name) != userDefinedTypes.end()) {
            return userDefinedTypes[name];
        }
        throw std::runtime_error("User-defined type not found: " + name);
    }

    void addTypeAlias(const std::string &alias, TypePtr type) { typeAliases[alias] = type; }

    TypePtr getTypeAlias(const std::string &alias)
    {
        if (typeAliases.find(alias) != typeAliases.end()) {
            return typeAliases[alias];
        }
        throw std::runtime_error("Type alias not found: " + alias);
    }

    TypePtr inferType(const ValuePtr &value) { return value->type; }

    bool checkType(const ValuePtr &value, const TypePtr &expectedType)
    {
        if (value->type->tag != expectedType->tag) {
            return false;
        }

        switch (expectedType->tag) {
        case TypeTag::Int:
        case TypeTag::Int8:
        case TypeTag::Int16:
        case TypeTag::Int32:
        case TypeTag::Int64:
        case TypeTag::UInt:
        case TypeTag::UInt8:
        case TypeTag::UInt16:
        case TypeTag::UInt32:
        case TypeTag::UInt64:
        case TypeTag::Float32:
        case TypeTag::Float64:
        case TypeTag::Bool:
        case TypeTag::String:
        case TypeTag::Nil:
            return true; // Simple types match by tag alone

        case TypeTag::List: {
            const auto &listType = std::get<ListType>(expectedType->extra);
            if (const auto *listValue = std::get_if<ListValue>(&value->data)) {
                for (const auto &element : listValue->elements) {
                    if (!checkType(element, listType.elementType)) {
                        return false;
                    }
                }
                return true;
            }
            break;
        }

        case TypeTag::Dict: {
            const auto &dictType = std::get<DictType>(expectedType->extra);
            if (const auto *dictValue = std::get_if<DictValue>(&value->data)) {
                for (const auto &[key, val] : dictValue->elements) {
                    if (!checkType(key, dictType.keyType) || !checkType(val, dictType.valueType)) {
                        return false;
                    }
                }
                return true;
            }
            break;
        }

        case TypeTag::Sum: {
            const auto &sumType = std::get<SumType>(expectedType->extra);
            if (const auto *sumValue = std::get_if<SumValue>(&value->data)) {
                if (sumValue->activeVariant >= sumType.variants.size()) {
                    return false;
                }
                const auto &variantType = sumType.variants[sumValue->activeVariant];
                return checkType(sumValue->value, variantType);
            }
            break;
        }

       case TypeTag::Enum: {
            if (const auto *enumType = std::get_if<EnumType>(&expectedType->extra)) {
                if (!enumType->values.empty()) {
                    value->data = EnumValue(enumType->values[0], expectedType);
                } else {
                    value->data = std::string(""); // Empty enum, use empty string as default
                }
            } else {
                throw std::runtime_error("Invalid enum type");
            }
            break;
        }

        case TypeTag::Function:
            // Function type checking might involve checking the signature
            // This is a placeholder and might need more complex logic
            return true;

        case TypeTag::Any:
            // Any type always matches
            return true;

            //default:
            //    throw std::runtime_error("Unsupported type tag: "
            //                             + std::to_string(static_cast<int>(expectedType->tag)));
        }

        // Handle Union type separately, as it can match multiple types
        if (expectedType->tag == TypeTag::Union) {
            const auto &unionType = std::get<UnionType>(expectedType->extra);
            for (const auto &type : unionType.types) {
                if (checkType(value, type)) {
                    return true;
                }
            }
            return false;
        }
        return false; // Fallback for non-matching types
    }

    ValuePtr convert(const ValuePtr &value, TypePtr targetType)
    {
        if (!isCompatible(value->type, targetType)) {
            throw std::runtime_error("Incompatible types: " + value->type->toString() + " and "
                                     + targetType->toString());
        }

        ValuePtr result = std::make_shared<Value>();
        result->type = targetType;

        std::visit(
            overloaded{[&](int64_t v) {
                           switch (targetType->tag) {
                           case TypeTag::Int:
                           case TypeTag::Int64:
                               result->data = v;
                               break;
                           case TypeTag::Int8:
                               result->data = safe_cast<int8_t>(v);
                               break;
                           case TypeTag::Int16:
                               result->data = safe_cast<int16_t>(v);
                               break;
                           case TypeTag::Int32:
                               result->data = safe_cast<int32_t>(v);
                               break;
                           case TypeTag::UInt:
                           case TypeTag::UInt64:
                               result->data = safe_cast<uint64_t>(v);
                               break;
                           case TypeTag::UInt8:
                               result->data = safe_cast<uint8_t>(v);
                               break;
                           case TypeTag::UInt16:
                               result->data = safe_cast<uint16_t>(v);
                               break;
                           case TypeTag::UInt32:
                               result->data = safe_cast<uint32_t>(v);
                               break;
                           case TypeTag::Float32:
                               result->data = safe_cast<float>(v);
                               break;
                           case TypeTag::Float64:
                               result->data = safe_cast<double>(v);
                               break;
                           case TypeTag::String:
                               result->data = std::to_string(v);
                               break;
                           default:
                               throw std::runtime_error("Unsupported conversion from int64_t to "
                                                        + targetType->toString());
                           }
                       },
                       [&](int32_t v) {
                           switch (targetType->tag) {
                           case TypeTag::Int:
                           case TypeTag::Int64:
                               result->data = safe_cast<int64_t>(v);
                               break;
                           case TypeTag::Int8:
                               result->data = safe_cast<int8_t>(v);
                               break;
                           case TypeTag::Int16:
                               result->data = safe_cast<int16_t>(v);
                               break;
                           case TypeTag::Int32:
                               result->data = v;
                               break;
                           case TypeTag::UInt:
                           case TypeTag::UInt64:
                               result->data = safe_cast<uint64_t>(v);
                               break;
                           case TypeTag::UInt8:
                               result->data = safe_cast<uint8_t>(v);
                               break;
                           case TypeTag::UInt16:
                               result->data = safe_cast<uint16_t>(v);
                               break;
                           case TypeTag::UInt32:
                               result->data = safe_cast<uint32_t>(v);
                               break;
                           case TypeTag::Float32:
                               result->data = safe_cast<float>(v);
                               break;
                           case TypeTag::Float64:
                               result->data = safe_cast<double>(v);
                               break;
                           case TypeTag::String:
                               result->data = std::to_string(v);
                               break;
                           default:
                               throw std::runtime_error("Unsupported conversion from int32_t to "
                                                        + targetType->toString());
                           }
                       },
                       [&](int16_t v) {
                           switch (targetType->tag) {
                           case TypeTag::Int:
                           case TypeTag::Int64:
                               result->data = safe_cast<int64_t>(v);
                               break;
                           case TypeTag::Int8:
                               result->data = safe_cast<int8_t>(v);
                               break;
                           case TypeTag::Int16:
                               result->data = v;
                               break;
                           case TypeTag::Int32:
                               result->data = safe_cast<int32_t>(v);
                               break;
                           case TypeTag::UInt:
                           case TypeTag::UInt64:
                               result->data = safe_cast<uint64_t>(v);
                               break;
                           case TypeTag::UInt8:
                               result->data = safe_cast<uint8_t>(v);
                               break;
                           case TypeTag::UInt16:
                               result->data = safe_cast<uint16_t>(v);
                               break;
                           case TypeTag::UInt32:
                               result->data = safe_cast<uint32_t>(v);
                               break;
                           case TypeTag::Float32:
                               result->data = safe_cast<float>(v);
                               break;
                           case TypeTag::Float64:
                               result->data = safe_cast<double>(v);
                               break;
                           case TypeTag::String:
                               result->data = std::to_string(v);
                               break;
                           default:
                               throw std::runtime_error("Unsupported conversion from int16_t to "
                                                        + targetType->toString());
                           }
                       },
                       [&](int8_t v) {
                           switch (targetType->tag) {
                           case TypeTag::Int:
                           case TypeTag::Int64:
                               result->data = safe_cast<int64_t>(v);
                               break;
                           case TypeTag::Int8:
                               result->data = v;
                               break;
                           case TypeTag::Int16:
                               result->data = safe_cast<int16_t>(v);
                               break;
                           case TypeTag::Int32:
                               result->data = safe_cast<int32_t>(v);
                               break;
                           case TypeTag::UInt:
                           case TypeTag::UInt64:
                               result->data = safe_cast<uint64_t>(v);
                               break;
                           case TypeTag::UInt8:
                               result->data = safe_cast<uint8_t>(v);
                               break;
                           case TypeTag::UInt16:
                               result->data = safe_cast<uint16_t>(v);
                               break;
                           case TypeTag::UInt32:
                               result->data = safe_cast<uint32_t>(v);
                               break;
                           case TypeTag::Float32:
                               result->data = safe_cast<float>(v);
                               break;
                           case TypeTag::Float64:
                               result->data = safe_cast<double>(v);
                               break;
                           case TypeTag::String:
                               result->data = std::to_string(v);
                               break;
                           default:
                               throw std::runtime_error("Unsupported conversion from int8_t to "
                                                        + targetType->toString());
                           }
                       },
                       [&](uint64_t v) {
                           switch (targetType->tag) {
                           case TypeTag::UInt:
                           case TypeTag::UInt64:
                               result->data = v;
                               break;
                           case TypeTag::UInt8:
                               result->data = safe_cast<uint8_t>(v);
                               break;
                           case TypeTag::UInt16:
                               result->data = safe_cast<uint16_t>(v);
                               break;
                           case TypeTag::UInt32:
                               result->data = safe_cast<uint32_t>(v);
                               break;
                           case TypeTag::Int:
                           case TypeTag::Int64:
                               result->data = safe_cast<int64_t>(v);
                               break;
                           case TypeTag::Int8:
                               result->data = safe_cast<int8_t>(v);
                               break;
                           case TypeTag::Int16:
                               result->data = safe_cast<int16_t>(v);
                               break;
                           case TypeTag::Int32:
                               result->data = safe_cast<int32_t>(v);
                               break;
                           case TypeTag::Float32:
                               result->data = safe_cast<float>(v);
                               break;
                           case TypeTag::Float64:
                               result->data = safe_cast<double>(v);
                               break;
                           case TypeTag::String:
                               result->data = std::to_string(v);
                               break;
                           default:
                               throw std::runtime_error("Unsupported conversion from uint64_t to "
                                                        + targetType->toString());
                           }
                       },
                       [&](uint32_t v) {
                           switch (targetType->tag) {
                           case TypeTag::UInt:
                           case TypeTag::UInt64:
                               result->data = safe_cast<uint64_t>(v);
                               break;
                           case TypeTag::UInt8:
                               result->data = safe_cast<uint8_t>(v);
                               break;
                           case TypeTag::UInt16:
                               result->data = safe_cast<uint16_t>(v);
                               break;
                           case TypeTag::UInt32:
                               result->data = v;
                               break;
                           case TypeTag::Int:
                           case TypeTag::Int64:
                               result->data = safe_cast<int64_t>(v);
                               break;
                           case TypeTag::Int8:
                               result->data = safe_cast<int8_t>(v);
                               break;
                           case TypeTag::Int16:
                               result->data = safe_cast<int16_t>(v);
                               break;
                           case TypeTag::Int32:
                               result->data = safe_cast<int32_t>(v);
                               break;
                           case TypeTag::Float32:
                               result->data = safe_cast<float>(v);
                               break;
                           case TypeTag::Float64:
                               result->data = safe_cast<double>(v);
                               break;
                           case TypeTag::String:
                               result->data = std::to_string(v);
                               break;
                           default:
                               throw std::runtime_error("Unsupported conversion from uint32_t to "
                                                        + targetType->toString());
                           }
                       },
                       [&](uint16_t v) {
                           switch (targetType->tag) {
                           case TypeTag::UInt:
                           case TypeTag::UInt64:
                               result->data = safe_cast<uint64_t>(v);
                               break;
                           case TypeTag::UInt8:
                               result->data = safe_cast<uint8_t>(v);
                               break;
                           case TypeTag::UInt16:
                               result->data = v;
                               break;
                           case TypeTag::UInt32:
                               result->data = safe_cast<uint32_t>(v);
                               break;
                           case TypeTag::Int:
                           case TypeTag::Int64:
                               result->data = safe_cast<int64_t>(v);
                               break;
                           case TypeTag::Int8:
                               result->data = safe_cast<int8_t>(v);
                               break;
                           case TypeTag::Int16:
                               result->data = safe_cast<int16_t>(v);
                               break;
                           case TypeTag::Int32:
                               result->data = safe_cast<int32_t>(v);
                               break;
                           case TypeTag::Float32:
                               result->data = safe_cast<float>(v);
                               break;
                           case TypeTag::Float64:
                               result->data = safe_cast<double>(v);
                               break;
                           case TypeTag::String:
                               result->data = std::to_string(v);
                               break;
                           default:
                               throw std::runtime_error("Unsupported conversion from uint16_t to "
                                                        + targetType->toString());
                           }
                       },
                       [&](uint8_t v) {
                           switch (targetType->tag) {
                           case TypeTag::UInt:
                           case TypeTag::UInt64:
                               result->data = safe_cast<uint64_t>(v);
                               break;
                           case TypeTag::UInt8:
                               result->data = v;
                               break;
                           case TypeTag::UInt16:
                               result->data = safe_cast<uint16_t>(v);
                               break;
                           case TypeTag::UInt32:
                               result->data = safe_cast<uint32_t>(v);
                               break;
                           case TypeTag::Int:
                           case TypeTag::Int64:
                               result->data = safe_cast<int64_t>(v);
                               break;
                           case TypeTag::Int8:
                               result->data = safe_cast<int8_t>(v);
                               break;
                           case TypeTag::Int16:
                               result->data = safe_cast<int16_t>(v);
                               break;
                           case TypeTag::Int32:
                               result->data = safe_cast<int32_t>(v);
                               break;
                           case TypeTag::Float32:
                               result->data = safe_cast<float>(v);
                               break;
                           case TypeTag::Float64:
                               result->data = safe_cast<double>(v);
                               break;
                           case TypeTag::String:
                               result->data = std::to_string(v);
                               break;
                           default:
                               throw std::runtime_error("Unsupported conversion from uint8_t to "
                                                        + targetType->toString());
                           }
                       },
                       [&](double v) {
                           switch (targetType->tag) {
                           case TypeTag::Float32:
                               result->data = safe_cast<float>(v);
                               break;
                           case TypeTag::Float64:
                               result->data = v;
                               break;
                           case TypeTag::Int:
                           case TypeTag::Int64:
                               result->data = safe_cast<int64_t>(v);
                               break;
                           case TypeTag::Int8:
                               result->data = safe_cast<int8_t>(v);
                               break;
                           case TypeTag::Int16:
                               result->data = safe_cast<int16_t>(v);
                               break;
                           case TypeTag::Int32:
                               result->data = safe_cast<int32_t>(v);
                               break;
                           case TypeTag::UInt:
                           case TypeTag::UInt64:
                               result->data = safe_cast<uint64_t>(v);
                               break;
                           case TypeTag::UInt8:
                               result->data = safe_cast<uint8_t>(v);
                               break;
                           case TypeTag::UInt16:
                               result->data = safe_cast<uint16_t>(v);
                               break;
                           case TypeTag::UInt32:
                               result->data = safe_cast<uint32_t>(v);
                               break;
                           case TypeTag::String:
                               result->data = std::to_string(v);
                               break;
                           default:
                               throw std::runtime_error("Unsupported conversion from double to "
                                                        + targetType->toString());
                           }
                       },
                       [&](const std::string &v) {
                           if (targetType->tag == TypeTag::String) {
                               result->data = v;
                           } else {
                               result = stringToNumber(v, targetType);
                           }
                       },
                       [&](bool v) {
                           if (targetType->tag == TypeTag::Bool) {
                               result->data = v;
                           } else if (targetType->tag == TypeTag::String) {
                               result->data = v ? "true" : "false";
                           } else {
                               throw std::runtime_error("Unsupported conversion from bool to "
                                                        + targetType->toString());
                           }
                       },
                       [&](const ListValue &lv) {
                           if (targetType->tag == TypeTag::List) {
                               result->data = lv;
                           } else {
                               throw std::runtime_error("Unsupported conversion from List to "
                                                        + targetType->toString());
                           }
                       },
                       [&](const DictValue &dv) {
                           if (targetType->tag == TypeTag::Dict) {
                               result->data = dv;
                           } else {
                               throw std::runtime_error("Unsupported conversion from Dict to "
                                                        + targetType->toString());
                           }
                       },
                       [&](const SumValue &sv) {
                           if (targetType->tag == TypeTag::Sum) {
                               result->data = sv;
                           } else {
                               throw std::runtime_error("Unsupported conversion from Sum to "
                                                        + targetType->toString());
                           }
                       },
                       [&](const UserDefinedValue &uv) {
                           if (targetType->tag == TypeTag::UserDefined) {
                               result->data = uv;
                           } else {
                               throw std::runtime_error(
                                   "Unsupported conversion from UserDefined to "
                                   + targetType->toString());
                           }
                       },
                       [&](std::monostate) {
                           if (targetType->tag == TypeTag::Nil) {
                               result->data = std::monostate{};
                           } else {
                               throw std::runtime_error("Unsupported conversion from Nil to "
                                                        + targetType->toString());
                           }
                       },
                       [&](auto) {
                           throw std::runtime_error("Unsupported conversion from type "
                                                    + value->type->toString() + " to "
                                                    + targetType->toString());
                       }},
            value->data);

        return result;
    }


};

