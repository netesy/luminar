//parser.hh
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

// Define a vector type to hold bytecode instructions
using Bytecode = std::vector<Instruction>;

// Forward declarations
class PrattParser;

// Function pointer type for Pratt parsing functions
using ParseFn = void (PrattParser::*)();

class PrattParser
{
public:
    // Constructor
    PrattParser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem);

    // Main parsing function
    Bytecode parse();
    std::string toString() const;                 //debug the parser
    std::vector<Instruction> getBytecode() const; //get the bytecode generated from the parser

private:
    std::vector<size_t> endJumps;
    std::vector<Token> tokens;
    bool hadError = false;
    size_t current = 0;                // get the current index position
    std::vector<Instruction> bytecode; // Declare bytecode as a local variable
    bool isNewExpression = true;
    // Scanner instance
    Scanner &scanner;

    //variables
    Variables variable; // Instance of Variables class
    //    std::unordered_set<std::string> variableMap;
    std::unordered_map<std::string, int> variableMap; // Use unordered_map to track variable indices
    int variableCounter = 0;                          // Initialize variable counter
    std::shared_ptr<TypeSystem> typeSystem;

    // Token variables
    Token currentToken;
    Token previousToken;

    // Pratt parsing functions (adapt from first parser or rewrite)
    void parsePrintStatement();      // print(), or debug() statements
    void parseIfStatement();         // if, elif , else statement
    void parseWhileLoop();           // while loop
    void parseForLoop();             //Python like forloop
    void parseMatchStatement();      // python like match and case
    void parseConcurrentStatement(); // Concurrent Operations
    void parseParallelStatement();   // Parallel Operations
    void parseFnDeclaration();       // Adapt from first parser (if supported)
    void parseFnCall();
    void parseClassDeclaration(); // Adapt from first parser (if supported)
    void parseReturnStatement();  // Adapt from first parser
    //To be implemented
    void parseImport();
    void parseModules();
    void parseTypes();

    // Pratt parsing utility functions
    ParseFn getParseFn(TokenType type);
    void parsePrecedence(Precedence precedence);
    Precedence getTokenPrecedence(TokenType type);

    // Token manipulation functions
    Token peek();
    Token peekNext();
    Token previous();
    void advance();

    void consume(TokenType type, const std::string &message);
    bool match(TokenType type);
    bool check(TokenType type);
    bool isAtEnd();
    bool isExpression(TokenType type);

    Instruction emit(Opcode opcode, uint32_t lineNumber);
    Instruction emit(Opcode opcode, uint32_t lineNumber, Value &&value);

    // Parse expression functions
    void parsePrimary();
    void parseExpression();
    void parseBinary();
    void parseLogical();
    void parseAnd();
    void parseOr();
    void parseEOF();
    void parseUnexpected();
    void parseComparison();
    void parseBoolean();
    void parseUnary();
    void parseLiteral();
    void parseString();
    void parseIf();
    void parseElseIf();
    void parseElse();
    void parseIdentifier();
    void parseDecVariable();
    void parseLoadVariable();
    void parseAssignment();
    void parseCall();

    // Parse statement functions
    void parseStatement();
    void parseExpressionStatement();
    void parseDeclaration();
    void parseBlock();
    void parseParenthesis();

    // Other helper functions
    void error(const std::string &message);

    //var methods
    void declareVariable(const Token &name,
                         const TypePtr &type,
                         std::optional<ValuePtr> defaultValue = std::nullopt)
    {
        try {
            int32_t memoryLocation = variable.addVariable(name.lexeme, type, false, defaultValue);
            emit(Opcode::DECLARE_VARIABLE,
                 name.line,
                 Value{std::make_shared<Type>(TypeTag::Int), memoryLocation});
        } catch (const std::runtime_error &e) {
            error(e.what());
        }
    }

    int32_t getVariableMemoryLocation(const Token &name)
    {
        try {
            return variable.getVariableMemoryLocation(name.lexeme);
        } catch (const std::runtime_error &e) {
            error(e.what());
            return -1; // Error case
        }
    }

    void enterScope() { variable.enterScope(); }

    void exitScope() { variable.exitScope(); }

    void synchronize();
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
                //            case TypeTag::Int64:
                //                value.data = std::stoll(input);
                //                break;
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
                // Assuming input is a comma-separated list of values
                {
                    //                    ListValue listValue;
                    //                    std::istringstream iss(input);
                    //                    std::string item;
                    //                    while (std::getline(iss, item, ',')) {
                    //                        listValue.elements.push_back(setValue(type->elementType, item));
                    //                    }
                    //                    value.data = listValue;
                }
                break;
            case TypeTag::Dict:
                // Assuming input is in the format "key1:value1,key2:value2"
                {
                    //                    DictValue dictValue;
                    //                    std::istringstream iss(input);
                    //                    std::string pair;
                    //                    while (std::getline(iss, pair, ',')) {
                    //                        size_t colonPos = pair.find(':');
                    //                        if (colonPos != std::string::npos) {
                    //                            std::string key = pair.substr(0, colonPos);
                    //                            std::string val = pair.substr(colonPos + 1);
                    //                            dictValue.elements[setValue(type->tag, key)] = setValue(type->tag, val);
                    //                        }
                    //                    }
                    //                    value.data = dictValue;
                }
                break;
            case TypeTag::Sum:
            case TypeTag::UserDefined:
                // These types might require more complex parsing logic
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

    TypeTag inferType(const Token &token)
    {
        //std::cout << "Current token: " << scanner.tokenTypeToString(token.type, token.lexeme)
        //<< std::endl;
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
            return TypeTag::Nil; // or create a Null type if needed
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
            return TypeTag::Any; // Default to Any for unknown types
        }
    }

    // Structure to hold string - TypeTag pairs
    struct TypeMapping
    {
        const char *str;
        TypeTag tag;
    };

    // Array of type mappings
    static constexpr std::array<TypeMapping, 23> typeMappings = {
        {{"int", TypeTag::Int},     {"i8", TypeTag::Int8},       {"i16", TypeTag::Int16},
         {"i32", TypeTag::Int32},   {"i64", TypeTag::Int64},     {"i128", TypeTag::Int64},
         {"uint", TypeTag::UInt},   {"u8", TypeTag::UInt8},      {"u16", TypeTag::UInt16},
         {"u32", TypeTag::UInt32},  {"u64", TypeTag::UInt64},    {"u128", TypeTag::UInt64},
         {"f32", TypeTag::Float32}, {"f64", TypeTag::Float64},   {"float", TypeTag::Float64},
         {"bool", TypeTag::Bool},   {"string", TypeTag::String}, {"dict", TypeTag::Dict},
         {"list", TypeTag::List},   {"enum", TypeTag::Enum},     {"any", TypeTag::Any}}};

    TypeTag stringToType(const std::string &typeStr)
    {
        auto it = std::find_if(typeMappings.begin(),
                               typeMappings.end(),
                               [&typeStr](const TypeMapping &mapping) {
                                   return typeStr == mapping.str;
                               });

        if (it != typeMappings.end()) {
            return it->tag;
        }

        return TypeTag::UserDefined;
    }
};
