#pragma once

#include "../instructions.hh"
#include "../scanner.hh"
#include "../types.hh"
#include "../variable.hh"
#include "algorithm.hh"
#include <any>
#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Define a vector type to hold bytecode instructions
using Bytecode = std::vector<Instruction>;

class PackratParser : public Algorithm
{
public:
    PackratParser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem);

    Bytecode parse() override;
    std::string toString() const override;
    std::vector<Instruction> getBytecode() const override;

private:
    std::vector<Token> tokens;
    size_t pos;
    bool hadError = false;
    Bytecode bytecode;
    Scanner &scanner;
    Variables variable;
    std::shared_ptr<TypeSystem> typeSystem;

    Instruction emit(Opcode opcode, uint32_t lineNumber);
    Instruction emit(Opcode opcode, uint32_t lineNumber, Value &&value);

    void declareVariable(const Token &name,
                         const TypePtr &type,
                         std::optional<ValuePtr> defaultValue = std::nullopt);
    int32_t getVariableMemoryLocation(const Token &name);
    void enterScope();
    void exitScope();

    void error(const std::string &message);

    Value setValue(TypePtr type, const std::string &input);
    TypeTag inferType(const Token &token);
    TypeTag stringToType(const std::string &typeStr);

    void program();
    void statement();
    void if_statement();
    void while_statement();
    void for_statement();
    void print_statement();
    void block();
    void handle_identifier();
    void var_declaration();
    void var_call(const Token &name);
    void assignment();
    void function_declaration();
    void function_call(const Token &name);
    void class_declaration();
    void method_call(const Token &name);
    void expression_statement();
    void expression();
    void parse_string();
    void interpolate_string(const std::string &str);
    std::vector<Token> tokenizeExpression(const std::string &expr);
    void range_function();
    void range_expression();
    void logical_or_expression();
    void logical_and_expression();
    void equality_expression();
    void comparison_expression();
    void additive_expression();
    void multiplicative_expression();
    void unary_expression();
    void primary_expression();

    Token peek();
    Token peekNext();
    Token previous();
    void parseTypes();
    void advance();
    void consume(TokenType type, const std::string &message);
    bool match(TokenType type);
    bool check(TokenType type);
    bool isAtEnd();
    bool isExpression(TokenType type);

    struct TypeMapping
    {
        const char *str;
        TypeTag tag;
    };

    // Define the static member typeMappings
    static constexpr std::array<TypeMapping, 23> typeMappings = {
        {{"int", TypeTag::Int},     {"i8", TypeTag::Int8},          {"i16", TypeTag::Int16},
         {"i32", TypeTag::Int32},   {"i64", TypeTag::Int64},        {"i128", TypeTag::Int64},
         {"uint", TypeTag::UInt},   {"u8", TypeTag::UInt8},         {"u16", TypeTag::UInt16},
         {"u32", TypeTag::UInt32},  {"u64", TypeTag::UInt64},       {"u128", TypeTag::UInt64},
         {"f32", TypeTag::Float32}, {"f64", TypeTag::Float64},      {"float", TypeTag::Float64},
         {"bool", TypeTag::Bool},   {"str", TypeTag::String},       {"dict", TypeTag::Dict},
         {"list", TypeTag::List},   {"enum", TypeTag::Enum},        {"any", TypeTag::Any},
         {"nil", TypeTag::Nil},     {"function", TypeTag::Function}}};
};
