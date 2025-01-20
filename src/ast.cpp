#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <variant>
#include "value.hh"  // Include the value system
#include "token.hh"

#ifndef AST_HH
#define AST_HH

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual TypeTag getType() const = 0;
    virtual ValuePtr evaluate() = 0;
    virtual std::string codegen() const {
        throw std::runtime_error("codegen() not implemented for this node type.");
    }
};

// Basic Literals
class NumberNode : public ASTNode {
private:
    ValuePtr value;

public:
    NumberNode(double num) {
        auto type = std::make_shared<Type>(TypeTag::Float64);
        value = std::make_shared<Value>(Value{type, num});
    }

    NumberNode(int64_t num) {
        auto type = std::make_shared<Type>(TypeTag::Int64);
        value = std::make_shared<Value>(Value{type, num});
    }

    TypeTag getType() const override { return TypeTag::NUMBER; }
    ValuePtr evaluate() override { return value; }

    std::string codegen() const override {
        return std::visit(overloaded{
                              [](double d) { return "push " + std::to_string(d) + "\n"; },
                              [](int64_t i) { return "push " + std::to_string(i) + "\n"; },
                              [](const auto&) { throw std::runtime_error("Invalid number type"); }
                          }, value->data);
    }
};

class StringLiteralNode : public ASTNode {
private:
    ValuePtr value;

public:
    StringLiteralNode(const std::string& str) {
        auto type = std::make_shared<Type>(TypeTag::String);
        value = std::make_shared<Value>(Value{type, str});
    }

    TypeTag getType() const override { return TypeTag::STRING; }
    ValuePtr evaluate() override { return value; }
};

class BoolNode : public ASTNode {
private:
    ValuePtr value;

public:
    BoolNode(bool b) {
        auto type = std::make_shared<Type>(TypeTag::Bool);
        value = std::make_shared<Value>(Value{type, b});
    }

    TypeTag getType() const override { return TypeTag::BOOL; }
    ValuePtr evaluate() override { return value; }
};

// Variables and Assignment
class VariableNode : public ASTNode {
private:
    std::string name;
    TypePtr type;
    bool mutable_;
    ValuePtr initialValue;

public:
    VariableNode(const std::string& name, TypePtr type, bool mutable_ = true, ValuePtr initialValue = nullptr)
        : name(name), type(type), mutable_(mutable_), initialValue(initialValue) {}

    TypeTag getType() const override { return TypeTag::VARIABLE; }
    ValuePtr evaluate() override { return initialValue; }

    const std::string& getName() const { return name; }
    bool isMutable() const { return mutable_; }
};

class AssignmentNode : public ASTNode {
private:
    std::unique_ptr<VariableNode> target;
    std::unique_ptr<ASTNode> value;

public:
    AssignmentNode(std::unique_ptr<VariableNode> target, std::unique_ptr<ASTNode> value)
        : target(std::move(target)), value(std::move(value)) {}

    TypeTag getType() const override { return TypeTag::ASSIGNMENT; }

    ValuePtr evaluate() override {
        if (!target->isMutable()) {
            throw std::runtime_error("Cannot assign to immutable variable");
        }
        return value->evaluate();
    }
};

// Block and Statements
class BlockNode : public ASTNode {
private:
    std::vector<std::unique_ptr<ASTNode>> statements;

public:
    BlockNode(std::vector<std::unique_ptr<ASTNode>> statements)
        : statements(std::move(statements)) {}

    TypeTag getType() const override { return TypeTag::BLOCK; }

    ValuePtr evaluate() override {
        ValuePtr lastValue = std::make_shared<Value>(Value{
            std::make_shared<Type>(TypeTag::Nil),
            std::monostate{}
        });

        for (const auto& stmt : statements) {
            lastValue = stmt->evaluate();
        }
        return lastValue;
    }
};

// Expressions
class BinaryNode : public ASTNode {
private:
    TokenType op;
    std::unique_ptr<ASTNode> left, right;

public:
    BinaryNode(TokenType op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    TypeTag getType() const override { return TypeTag::BINARY; }

    ValuePtr evaluate() override {
        ValuePtr left_val = left->evaluate();
        ValuePtr right_val = right->evaluate();

        auto createNumericResult = [](TypeTag tag, auto value) {
            auto type = std::make_shared<Type>(tag);
            return std::make_shared<Value>(Value{type, value});
        };

        return std::visit(overloaded{
                              [&](double lhs, double rhs) {
                                  switch (op) {
                                  case TokenType::PLUS: return createNumericResult(TypeTag::Float64, lhs + rhs);
                                  case TokenType::MINUS: return createNumericResult(TypeTag::Float64, lhs - rhs);
                                  case TokenType::STAR: return createNumericResult(TypeTag::Float64, lhs * rhs);
                                  case TokenType::SLASH: {
                                      if (rhs == 0.0) throw std::runtime_error("Division by zero");
                                      return createNumericResult(TypeTag::Float64, lhs / rhs);
                                  }
                                  default: throw std::runtime_error("Invalid operator for float operands");
                                  }
                              },
                              [&](int64_t lhs, int64_t rhs) {
                                  switch (op) {
                                  case TokenType::PLUS: return createNumericResult(TypeTag::Int64, lhs + rhs);
                                  case TokenType::MINUS: return createNumericResult(TypeTag::Int64, lhs - rhs);
                                  case TokenType::STAR: return createNumericResult(TypeTag::Int64, lhs * rhs);
                                  case TokenType::SLASH: {
                                      if (rhs == 0) throw std::runtime_error("Division by zero");
                                      return createNumericResult(TypeTag::Int64, lhs / rhs);
                                  }
                                  case TokenType::MODULUS: return createNumericResult(TypeTag::Int64, lhs % rhs);
                                  default: throw std::runtime_error("Invalid operator for integer operands");
                                  }
                              },
                              [&](const std::string& lhs, const std::string& rhs) {
                                  if (op == TokenType::PLUS) {
                                      auto type = std::make_shared<Type>(TypeTag::String);
                                      return std::make_shared<Value>(Value{type, lhs + rhs});
                                  }
                                  throw std::runtime_error("Invalid operator for string operands");
                              },
                              [](const auto&, const auto&) {
                                  throw std::runtime_error("Type mismatch in binary operation");
                              }
                          }, left_val->data, right_val->data);
    }
};

class UnaryNode : public ASTNode {
private:
    TokenType op;
    std::unique_ptr<ASTNode> expr;

public:
    UnaryNode(TokenType op, std::unique_ptr<ASTNode> expr)
        : op(op), expr(std::move(expr)) {}

    TypeTag getType() const override { return TypeTag::UNARY; }

    ValuePtr evaluate() override {
        ValuePtr operand = expr->evaluate();

        return std::visit(overloaded{
                              [&](double val) {
                                  auto type = std::make_shared<Type>(TypeTag::Float64);
                                  switch(op) {
                                  case TokenType::MINUS: return std::make_shared<Value>(Value{type, -val});
                                  case TokenType::PLUS: return std::make_shared<Value>(Value{type, val});
                                  default: throw std::runtime_error("Invalid unary operator for float");
                                  }
                              },
                              [&](int64_t val) {
                                  auto type = std::make_shared<Type>(TypeTag::Int64);
                                  switch(op) {
                                  case TokenType::MINUS: return std::make_shared<Value>(Value{type, -val});
                                  case TokenType::PLUS: return std::make_shared<Value>(Value{type, val});
                                  default: throw std::runtime_error("Invalid unary operator for integer");
                                  }
                              },
                              [](const auto&) { throw std::runtime_error("Invalid type for unary operation"); }
                          }, operand->data);
    }
};

// Control Flow
class ConditionalNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> thenBranch;
    std::unique_ptr<ASTNode> elseBranch;

public:
    ConditionalNode(std::unique_ptr<ASTNode> condition,
                    std::unique_ptr<ASTNode> thenBranch,
                    std::unique_ptr<ASTNode> elseBranch)
        : condition(std::move(condition))
        , thenBranch(std::move(thenBranch))
        , elseBranch(std::move(elseBranch)) {}

    TypeTag getType() const override { return TypeTag::CONDITIONAL; }

    ValuePtr evaluate() override {
        ValuePtr cond_val = condition->evaluate();
        bool is_true = std::visit(overloaded{
                                      [](bool b) { return b; },
                                      [](int64_t i) { return i != 0; },
                                      [](double d) { return d != 0.0; },
                                      [](const auto&) { return false; }
                                  }, cond_val->data);

        return is_true ? thenBranch->evaluate() : elseBranch->evaluate();
    }
};

// Pattern Matching
class PatternMatchNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> expr;
    std::vector<std::pair<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>>> patterns;
    std::unique_ptr<ASTNode> defaultBranch;

public:
    PatternMatchNode(std::unique_ptr<ASTNode> expr,
                     std::vector<std::pair<std::unique_ptr<ASTNode>,
                                           std::unique_ptr<ASTNode>>> patterns,
                     std::unique_ptr<ASTNode> defaultBranch = nullptr)
        : expr(std::move(expr))
        , patterns(std::move(patterns))
        , defaultBranch(std::move(defaultBranch)) {}

    TypeTag getType() const override { return TypeTag::PATTERN_MATCH; }

    ValuePtr evaluate() override {
        ValuePtr matchVal = expr->evaluate();

        for (const auto& [pattern, result] : patterns) {
            if (doesMatch(pattern->evaluate(), matchVal)) {
                return result->evaluate();
            }
        }

        return defaultBranch ? defaultBranch->evaluate() :
                   std::make_shared<Value>(Value{
                       std::make_shared<Type>(TypeTag::Nil),
                       std::monostate{}
                   });
    }

private:
    bool doesMatch(const ValuePtr& pattern, const ValuePtr& value) {
        return std::visit(overloaded{
                              [](const EnumValue& p, const EnumValue& v) {
                                  return p.variantName == v.variantName;
                              },
                              [](const auto& p, const auto& v) { return p == v; }
                          }, pattern->data, value->data);
    }
};

// Collections
class ListNode : public ASTNode {
private:
    std::vector<std::unique_ptr<ASTNode>> elements;

public:
    ListNode(std::vector<std::unique_ptr<ASTNode>> elements)
        : elements(std::move(elements)) {}

    TypeTag getType() const override { return TypeTag::LIST; }

    ValuePtr evaluate() override {
        auto type = std::make_shared<Type>(TypeTag::List);
        ListValue listValue;

        for (const auto& element : elements) {
            listValue.append(element->evaluate());
        }

        return std::make_shared<Value>(Value{type, listValue});
    }
};

class DictNode : public ASTNode {
private:
    std::vector<std::pair<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>>> entries;

public:
    DictNode(std::vector<std::pair<std::unique_ptr<ASTNode>, std::unique_ptr<ASTNode>>> entries)
        : entries(std::move(entries)) {}

    TypeTag getType() const override { return TypeTag::DICT; }

    ValuePtr evaluate() override {
        auto type = std::make_shared<Type>(TypeTag::Dict);
        DictValue dictValue;

        for (const auto& [key, value] : entries) {
            dictValue.insert(key->evaluate(), value->evaluate());
        }

        return std::make_shared<Value>(Value{type, dictValue});
    }
};

// Functions and Classes
class FunctionNode : public ASTNode {
private:
    std::string name;
    std::vector<std::pair<std::string, TypePtr>> parameters;
    TypePtr returnType;
    std::unique_ptr<BlockNode> body;

public:
    FunctionNode(const std::string& name,
                 std::vector<std::pair<std::string, TypePtr>> parameters,
                 TypePtr returnType,
                 std::unique_ptr<BlockNode> body)
        : name(name)
        , parameters(std::move(parameters))
        , returnType(std::move(returnType))
        , body(std::move(body)) {}

    TypeTag getType() const override { return TypeTag::FUNCTION; }

    ValuePtr evaluate() override {
        auto type = std::make_shared<Type>(TypeTag::Function);
        FunctionType funcType;
        for (const auto& param : parameters) {
            funcType.paramTypes.push_back(param.second);
        }
        funcType.returnType = returnType;
        type->extra = funcType;

        return std::make_shared<Value>(Value{type,
                                             FunctionValue{name, parameters, returnType, body.get()}});
    }
};

class ClassNode : public ASTNode {
private:
    std::string name;
    std::vector<std::unique_ptr<VariableNode>> fields;
    std::vector<std::unique_ptr<FunctionNode>> methods;
    std::unique_ptr<FunctionNode> constructor;

public:
    ClassNode(const std::string& name,
              std::vector<std::unique_ptr<VariableNode>> fields,
              std::vector<std::unique_ptr<FunctionNode>> methods,
              std::unique_ptr<FunctionNode> constructor = nullptr)
        : name(name)
        , fields(std::move(fields))
        , methods(std::move(methods))
        , constructor(std::move(constructor)) {}

    TypeTag getType() const override { return TypeTag::CLASS;
