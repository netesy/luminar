#pragma once

#include "../instructions.hh"
#include "../precedence.hh"
#include "../scanner.hh"
#include "../types.hh"
#include "../variable.hh"
#include <any>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "../ast.hh"

using Bytecode = std::vector<Instruction>;

// Forward declarations
class PrattParser;

// Define function pointer types for the parser
typedef std::unique_ptr<ASTNode> (PrattParser::*PrefixParseFn)(Token);
typedef std::unique_ptr<ASTNode> (PrattParser::*InfixParseFn)(std::unique_ptr<ASTNode>, Token);

class PrattParser {
public:
    PrattParser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem);
    Bytecode parse();
    std::string toString() const;

private:
    // Core parsing methods
    std::unique_ptr<ASTNode> parseExpression(Precedence precedence);
    std::unique_ptr<ASTNode> parseDeclaration();
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseExpressionStatement();
    std::unique_ptr<ASTNode> createEmptyNode(); // Added from commented code

    // Added specialized parsers from commented code
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseLogical();
    std::unique_ptr<ASTNode> parseComparison();
    std::unique_ptr<ASTNode> parseString();
    std::unique_ptr<ASTNode> parseIf();
    std::unique_ptr<ASTNode> parseBlock();
    std::unique_ptr<ASTNode> parseReturnStatement();

    // Added specialized statement parsers from commented code
    std::unique_ptr<ASTNode> parseConcurrentStatement();
    std::unique_ptr<ASTNode> parseParallelStatement();
    std::unique_ptr<ASTNode> parseImport();
    std::unique_ptr<ASTNode> parseModules();
    void parseTypes();
    std::unique_ptr<ASTNode> parseUnexpected();
    std::unique_ptr<ASTNode> parseParenthesis();

    // Function retrieval methods
    PrefixParseFn getPrefixParseFn(TokenType type);
    InfixParseFn getInfixParseFn(TokenType type);

    // Utility methods
    Token advance();
    Token peek();
    Token peekNext();
    bool isAtEnd();
    Token previous();
    bool check(TokenType type);
    bool match(TokenType type);
    void consume(TokenType type, const std::string &message);
    Precedence getTokenPrecedence(TokenType type);
    void error(const std::string &message);
    void synchronize();
    bool isExpression(TokenType type); // Added from commented code

    // Bytecode emission methods
    Instruction emit(Opcode opcode, uint32_t lineNumber);
    Instruction emit(Opcode opcode, uint32_t lineNumber, Value &&value);

    // Variable management - Added from commented code
    void declareVariable(const Token &name,
                         const TypePtr &type,
                         std::optional<ValuePtr> defaultValue = std::nullopt);
    int32_t getVariableMemoryLocation(const Token &name);
    void enterScope() { variable.enterScope(); }
    void exitScope() { variable.exitScope(); }
    void parseLoadVariable(); // Added from commented code
    void parseDecVariable();  // Already had this, but keeping for consistency

    // Type inference and handling - Added from commented code
    TypeTag inferType(const Token &token)
    {
        switch (token.type) {
        case TokenType::NUMBER:
            // Check if the number contains a decimal point
            if (token.lexeme.find('.') != std::string::npos) {
                return TypeTag::Float64;
            } else {
                return TypeTag::Int;
            }
        case TokenType::STRING:
            return TypeTag::String;
        case TokenType::TRUE:
        case TokenType::FALSE:
            return TypeTag::Bool;
        case TokenType::NIL_TYPE:
            return TypeTag::Nil;
        case TokenType::INT_TYPE:
            return TypeTag::Int;
        case TokenType::INT8_TYPE:
            return TypeTag::Int8;
        case TokenType::INT16_TYPE:
            return TypeTag::Int16;
        case TokenType::INT32_TYPE:
            return TypeTag::Int32;
        case TokenType::INT64_TYPE:
            return TypeTag::Int64;
        case TokenType::UINT_TYPE:
            return TypeTag::UInt;
        case TokenType::UINT8_TYPE:
            return TypeTag::UInt8;
        case TokenType::UINT16_TYPE:
            return TypeTag::UInt16;
        case TokenType::UINT32_TYPE:
            return TypeTag::UInt32;
        case TokenType::UINT64_TYPE:
            return TypeTag::UInt64;
        case TokenType::FLOAT32_TYPE:
            return TypeTag::Float32;
        case TokenType::FLOAT_TYPE:
        case TokenType::FLOAT64_TYPE:
            return TypeTag::Float64;
        case TokenType::SUM_TYPE:
            return TypeTag::Sum;
        case TokenType::ANY_TYPE:
            return TypeTag::Any;
        case TokenType::UNION_TYPE:
            return TypeTag::Union;
        case TokenType::USER_TYPE:
            return TypeTag::UserDefined;
        case TokenType::BOOL_TYPE:
            return TypeTag::Bool;
        case TokenType::FUNCTION_TYPE:
            return TypeTag::Function;
        default:
            return TypeTag::Any;
        }
    }
    TypeTag stringToType(const std::string &typeStr);
    Value setValue(TypePtr type, const std::string &input)
    {
        try {
            Value value;
            value.type = type;

            switch (type->tag) {
            case TypeTag::Bool:
                if (input == "true")
                    value.data = true;
                else if (input == "false")
                    value.data = false;
                else
                    throw std::runtime_error("Invalid boolean value: " + input);
                break;
            case TypeTag::Int:
            case TypeTag::Int64:
                value.data = static_cast<int64_t>(std::stoll(input));
                break;
            case TypeTag::Int8:
                value.data = static_cast<int8_t>(std::stol(input));
                break;
            case TypeTag::Int16:
                value.data = static_cast<int16_t>(std::stol(input));
                break;
            case TypeTag::Int32:
                value.data = static_cast<int32_t>(std::stol(input));
                break;
            case TypeTag::UInt:
            case TypeTag::UInt64:
                value.data = static_cast<uint64_t>(std::stoull(input));
                break;
            case TypeTag::UInt8:
                value.data = static_cast<uint8_t>(std::stoull(input));
                break;
            case TypeTag::UInt16:
                value.data = static_cast<uint16_t>(std::stoull(input));
                break;
            case TypeTag::UInt32:
                value.data = static_cast<uint32_t>(std::stoull(input));
                break;
            case TypeTag::Float32:
                value.data = std::stof(input);
                break;
            case TypeTag::Float64:
                value.data = std::stod(input);
                break;
            case TypeTag::String:
                value.data = input;
                break;
            case TypeTag::List:
                // List handling is commented out in original code
                throw std::runtime_error("List type not fully implemented.");
                break;
            case TypeTag::Dict:
                // Dict handling is commented out in original code
                throw std::runtime_error("Dict type not fully implemented.");
                break;
            case TypeTag::Sum:
            case TypeTag::UserDefined:
                throw std::runtime_error(
                    "Sum and UserDefined types are not supported in this setValue function");
            default:
                throw std::runtime_error("Unsupported type for value setting: " + type->toString());
            }

            return value;
        } catch (const std::exception &e) {
            throw std::runtime_error("Failed to set value: " + std::string(e.what()));
        }
    }

    // Prefix parse functions (handle tokens that start an expression)
    std::unique_ptr<ASTNode> parseLiteral(Token token);
    std::unique_ptr<ASTNode> parseBoolean(Token token);
    std::unique_ptr<ASTNode> parseUnary(Token token);
    std::unique_ptr<ASTNode> parseGrouping(Token token);
    std::unique_ptr<ASTNode> parseIdentifier(Token token);
    std::unique_ptr<ASTNode> parseDecVariable(Token token);
    std::unique_ptr<ASTNode> parseFnDeclaration(Token token);

    // Infix parse functions (handle tokens that appear after a left operand)
    std::unique_ptr<ASTNode> parseBinaryOp(std::unique_ptr<ASTNode> left, Token token);
    std::unique_ptr<ASTNode> parseAndOp(std::unique_ptr<ASTNode> left, Token token);
    std::unique_ptr<ASTNode> parseOrOp(std::unique_ptr<ASTNode> left, Token token);
    std::unique_ptr<ASTNode> parseAssignment(std::unique_ptr<ASTNode> left, Token token);
    std::unique_ptr<ASTNode> parseCall(std::unique_ptr<ASTNode> left, Token token);

    // Statement parse functions
    std::unique_ptr<ASTNode> parseBlock(Token token);
   // std::unique_ptr<ASTNode> parsePrintStatement(Token token);
    std::unique_ptr<ASTNode> parseIfStatement(Token token);
    std::unique_ptr<ASTNode> parseWhileLoop(Token token);
    std::unique_ptr<ASTNode> parseForLoop(Token token);
    std::unique_ptr<ASTNode> parseMatchStatement(Token token);
    std::unique_ptr<ASTNode> parseClassDeclaration(Token token);
    std::unique_ptr<ASTNode> parseFnCall(); // Added from commented code
    std::unique_ptr<ASTNode> parseEOF();    // Added from commented code

    // Member variables
    Scanner &scanner;
    std::shared_ptr<TypeSystem> typeSystem;
    std::vector<Token> tokens;
    size_t current = 0;
    Bytecode bytecode;
    std::vector<std::unique_ptr<ASTNode>> ast;
    bool hadError = false;
    bool isNewExpression = true;

    // Additional member variables from commented code
    std::vector<size_t> endJumps;
    Token currentToken;
    Token previousToken;
    Variables variable;
    std::unordered_map<std::string, int> variableMap;
    int variableCounter = 0;
    ASTNode* currentNode = nullptr; // Track the current node

    // Type mapping structure from commented code
    struct TypeMapping {
        const char *str;
        TypeTag tag;
    };

    // Array of type mappings from commented code
    static constexpr std::array<TypeMapping, 23> typeMappings = {
        {{"int", TypeTag::Int},     {"i8", TypeTag::Int8},       {"i16", TypeTag::Int16},
         {"i32", TypeTag::Int32},   {"i64", TypeTag::Int64},     {"i128", TypeTag::Int64},
         {"uint", TypeTag::UInt},   {"u8", TypeTag::UInt8},      {"u16", TypeTag::UInt16},
         {"u32", TypeTag::UInt32},  {"u64", TypeTag::UInt64},    {"u128", TypeTag::UInt64},
         {"f32", TypeTag::Float32}, {"f64", TypeTag::Float64},   {"float", TypeTag::Float64},
         {"bool", TypeTag::Bool},   {"string", TypeTag::String}, {"dict", TypeTag::Dict},
         {"list", TypeTag::List},   {"enum", TypeTag::Enum},     {"any", TypeTag::Any}}};
};
