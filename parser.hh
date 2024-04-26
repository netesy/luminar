//parser.hh
#include "opcodes.hh"
#include "scanner.hh"
#include "symbol.hh"
#include <any>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

enum ReturnType { VOID, INT, FLOAT, BOOL, STRING, DICT, LIST };
// Define a vector type to hold bytecode instructions
using Bytecode = std::vector<Instruction>;

// Forward declarations
class Parser;

// Function pointer type for Pratt parsing functions
using ParseFn = void (Parser::*)();

enum Precedence {
    PREC_NONE,         // The lowest precedence, used for non-operators
    PREC_ASSIGNMENT,   // Assignment operators: =, +=, -=, *=, /=
    PREC_OR,           // Logical OR operator: or
    PREC_AND,          // Logical AND operator: and
    PREC_EQUALITY,     // Equality operators: ==, !=
    PREC_COMPARISON,   // Comparison operators: <, >, <=, >=
    PREC_TERM,         // Addition and subtraction: +, -
    PREC_FACTOR,       // Multiplication and division: *, /
    PREC_UNARY,        // Unary operators: !, -
    PREC_CALL,         // Function or method call: . ()
    PREC_PRIMARY       // The highest precedence, used for primary expressions
};

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
    std::vector<Token> tokens;
    size_t current = 0;                           // get the current index position
    std::vector<Instruction> bytecode;            // Declare bytecode as a local variable

    // Scanner instance
    Scanner &scanner;

    // Symbol table
    SymbolTable symbolTable;

    // Token variables
    Token currentToken;
    Token previousToken;

    // Pratt parsing functions (adapt from first parser or rewrite)
    void parsePrintStatement();      // print(), or debug() statements
    void parseIfStatement();         // if, elif , else statement
    void parseWhileLoop();           // while loop
    void parseForLoop();             //Python like forloop
    void parseMatchStatement();      // python like match and case
    void parseFunctionDeclaration(); // Adapt from first parser (if supported)
    void parseClassDeclaration();    // Adapt from first parser (if supported)
    void parseReturnStatement();     // Adapt from first parser

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
    bool isExpressionStart(TokenType type);
    bool isBinaryOperator(TokenType type);

    Instruction makeInstruction(Opcode opcode, uint32_t lineNumber);
    Instruction makeInstruction(Opcode opcode,
                                uint32_t lineNumber,
                                std::variant<int32_t, double, bool, std::string> value);

    // Parse expression functions
    void parsePrimary();
    void parseExpression();
    void parseBinary();
    void parseLogical();
    void parseAnd();
    void parseOr();
    void parseComparison();
    void parseBoolean();
    void parseUnary();
    void parseLiteral();
    void parseVariable();
    void parseAssignment();
    void parseCall();

    // Parse statement functions
    void parseStatement();
    void parseBlock();
    void parseParenthesis();

    // Other helper functions (adapt from first parser or rewrite)
    void error(const std::string &message);
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
