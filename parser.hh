//parser.hh
#include "instructions.hh"
#include "precedence.hh"
#include "scanner.hh"
#include "scope.hh"
#include "variable.hh"
#include <any>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Define a vector type to hold bytecode instructions
using Bytecode = std::vector<Instruction>;

// Forward declarations
class Parser;

// Function pointer type for Pratt parsing functions
using ParseFn = void (Parser::*)();

class Parser
{
public:
    // Constructor
    Parser(Scanner &scanner);

    // Main parsing function
    Bytecode parse();
    std::string toString() const;                 //debug the parser
    std::vector<Instruction> getBytecode() const; //get the bytecode generated from the parser

private:
    std::vector<size_t> endJumps;
    std::vector<Token> tokens;
    bool hadError = false;
    size_t current = 0;                           // get the current index position
    std::vector<Instruction> bytecode;            // Declare bytecode as a local variable
    bool isNewExpression = true;
    // Scanner instance
    Scanner &scanner;

    //variables 
    Variables variable; // Instance of Variables class
    //    std::unordered_set<std::string> variableMap;
    std::unordered_map<std::string, int> variableMap; // Use unordered_map to track variable indices
    int variableCounter = 0;                          // Initialize variable counter

    // Token variables
    Token currentToken;
    Token previousToken;

    // Pratt parsing functions (adapt from first parser or rewrite)
    void parsePrintStatement();      // print(), or debug() statements
    void parseIfStatement();         // if, elif , else statement
    void parseWhileLoop();           // while loop
    void parseForLoop();             //Python like forloop
    void parseMatchStatement();      // python like match and case
    void parseConcurrentStatement();      // Concurrent Operations
    void parseParallelStatement();      // Parallel Operations
    void parseFnDeclaration();       // Adapt from first parser (if supported)
    void parseFnCall();
    void parseClassDeclaration();    // Adapt from first parser (if supported)
    void parseReturnStatement();     // Adapt from first parser
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
    Instruction emit(Opcode opcode,
                                uint32_t lineNumber,
                                std::variant<int32_t, double, bool, std::string> value);

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
            emit(Opcode::DECLARE_VARIABLE, name.line, memoryLocation);
        } catch (const std::runtime_error& e) {
            error(e.what());
        }
    }

    int32_t getVariableMemoryLocation(const Token& name) {
        try {
            return variable.getVariableMemoryLocation(name.lexeme);
        } catch (const std::runtime_error& e) {
            error(e.what());
            return -1; // Error case
        }
    }

    void enterScope() {
        variable.enterScope();
    }

    void exitScope() { variable.exitScope(); }

    void synchronize();

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

//// Maybe consider using references for efficiency
//class Variable
//{
//public:
//  Type type;
//  int index;

//  Variable(Type type, int index)
//      : type(type), index(index) {}
//};

//// Maybe consider using references for efficiency
//class Function
//{
//public:
//  Type returnType;
//  std::vector<Variable> parameters;

//  Function(Type returnType, const std::vector<Variable> &parameters)
//      : returnType(returnType), parameters(parameters) {}
//};

//// Maybe consider using references for efficiency
//class Class
//{
//public:
//  std::string className;
//  std::vector<Variable> fields;
//  FunctionTable methods;

//  Class(const std::string &className,
//        const std::vector<Variable> &fields,
//        const FunctionTable &methods)
//      : className(className), fields(fields), methods(methods) {}
//};
