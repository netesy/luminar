// types.hh
#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

class BaseType
{
public:
    virtual ~BaseType() = default;
    virtual std::string getTypeName() const = 0;
};

// Forward declaration of Type
class IntegerType;
class FloatType;
class StringType;
class BooleanType;
class UserType;
class FunctionType;
class ListType;
class DictType;
class ArrayType;
class EnumType;

using Type = std::variant < std::shared_ptr<IntegerType>;
std::shared_ptr<FloatType>, std::shared_ptr<StringType>, std::shared_ptr<BooleanType>,
    std::shared_ptr<UserType>, std::shared_ptr<FunctionType>, std::shared_ptr<ListType>,
    std::shared_ptr<DictType>, std::shared_ptr<ArrayType>, std::shared_ptr < EnumType >> ;

class IntegerType : public BaseType
{
public:
    std::string getTypeName() const override { return "int"; }
};

class FloatType : public BaseType
{
public:
    std::string getTypeName() const override { return "float"; }
};

class StringType : public BaseType
{
public:
    std::string getTypeName() const override { return "str"; }
};

class BooleanType : public BaseType
{
public:
    std::string getTypeName() const override { return "bool"; }
};

class UserType : public BaseType
{
public:
    std::string typeName;

    std::string getTypeName() const override { return typeName; }
};

class FunctionType : public BaseType
{
public:
    std::string returnType;
    std::vector<std::string> parameterTypes;

    std::string getTypeName() const override { return "fn"; }
};

class ListType : public BaseType
{
public:
    Type elementType; // Type of elements in the list

    std::string getTypeName() const override { return "list"; }
};

class DictType : public BaseType
{
public:
    Type keyType;   // Type of keys in the dictionary
    Type valueType; // Type of values in the dictionary

    std::string getTypeName() const override { return "dict"; }
};

class ArrayType : public BaseType
{
public:
    Type elementType; // Type of elements in the array
    size_t size;      // Size of the array

    std::string getTypeName() const override { return "array"; }
};

class EnumType : public BaseType
{
public:
    std::vector<std::string> enumValues; // Names of enumeration values

    std::string getTypeName() const override { return "enum"; }
};
