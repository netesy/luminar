//parser.hh
#include "opcodes.hh"
#include "scanner.hh"
#include <any>
#include <string>
#include <unordered_map>

enum class ReturnType { VOID, INT, FLOAT, BOOL, STRING };
// Define a vector type to hold bytecode instructions
using Bytecode = std::vector<Instruction>;

struct ParameterInfo
{
    std::string name;
    std::any type;
};

// Define the FunctionInfo struct with returnType field
struct FunctionInfo
{
    std::string name;
    uint32_t memoryLocation;
    std::vector<std::string> parameters;
    // std::string returnType; // New field to store the return type
    // ReturnType returnType;

    // Constructors if needed
    FunctionInfo(uint32_t memLoc,
                 const std::vector<std::string> &params,
                 const std::string &retType = "")
        : memoryLocation(memLoc)
        , parameters(params)
    //, returnType(retType)
    {}
    FunctionInfo() = default;
};

class SymbolTable
{
public:
    void declareVariable(const std::string &name, uint32_t memoryLocation)
    {
        variables[name] = memoryLocation;
    }

    uint32_t getVariableMemoryLocation(const std::string &name) const
    {
        if (variables.count(name)) {
            return variables.at(name);
        }
        throw std::runtime_error("Variable not found in symbol table.");
    }

private:
    std::unordered_map<std::string, uint32_t> variables;
};

class Parser
{
public:
    explicit Parser(Scanner &scanner);

    Bytecode parse();
    std::string toString() const;
    size_t current = 0;

private:
    Scanner &scanner;
    std::vector<Token> tokens;
    uint32_t nextMemoryLocation;
    SymbolTable symbolTable;

    std::vector<Instruction> bytecode; // Declare bytecode as a local variable
    std::unordered_map<std::string, FunctionInfo> functionTable;
    // Add a symbol table to store function names and their memory locations
    std::unordered_map<std::string, uint32_t> functionSymbolTable;
    std::unordered_map<std::string, std::vector<std::string>> functionParametersTable;

    void declareFunction(const std::string &name, const FunctionInfo &info)
    {
        functionTable[name] = info;
        // functionParametersTable[name];
    }

    FunctionInfo getFunctionInfo(const std::string &name) const
    {
        FunctionInfo functionInfo;
        functionInfo.name = name;
        functionInfo.memoryLocation = getFunctionMemoryLocation(name);

        // Get function parameters
        // functionInfo.parameters = functionParametersTable[name];

        //        // Get function return type
        //        const Token &returnTypeToken = peekNext();
        //        if (returnTypeToken.type == TokenType::INT_TYPE) {
        //            functionInfo.returnType = ReturnType::INT;
        //        } else if (returnTypeToken.type == TokenType::FLOAT_TYPE) {
        //            functionInfo.returnType = ReturnType::FLOAT;
        //        } else if (returnTypeToken.type == TokenType::BOOL_TYPE) {
        //            functionInfo.returnType = ReturnType::BOOL;
        //        } else if (returnTypeToken.type == TokenType::STR_TYPE) {
        //            functionInfo.returnType = ReturnType::STRING;
        //        } else {
        //            functionInfo.returnType = ReturnType::VOID; // Default return type
        //        }

        return functionInfo;
    }

    // Helper function to get the memory location of a function from the symbol table
    uint32_t getFunctionMemoryLocation(const std::string &name) const;

    void advance();
    Token peek();     // Changed to return non-const reference
    Token previous(); // Changed to return non-const reference
    Token peekNext();

    bool match(TokenType type);
    void consume(TokenType type, const std::string &message);
    void error(const std::string &message);

    Instruction makeInstruction(Opcode opcode, uint32_t lineNumber);
    Instruction makeInstruction(Opcode opcode, uint32_t lineNumber, float floatValue);
    Instruction makeInstruction(Opcode opcode, uint32_t lineNumber, std::string value);
    Instruction makeInstruction(Opcode opcode, uint32_t lineNumber, int32_t intValue);
    Instruction makeInstruction(Opcode opcode, uint32_t lineNumber, bool boolValue);

    Bytecode block();
    Bytecode statement();
    Bytecode importStatement();
    Bytecode ifStatement();
    Bytecode whileStatement();
    Bytecode expressionStatement();
    Bytecode expression();
    Bytecode conditional();
    Bytecode logicalOr();
    Bytecode logicalAnd();
    Bytecode equality();
    Bytecode comparison();
    Bytecode addition();
    Bytecode multiplication();
    Bytecode unary();
    Bytecode assignment();
    Bytecode primary();
    void varDeclaration();
    void varInvoke();
    Bytecode functionDeclaration();
    Bytecode functionCall(const std::string &functionName);
    std::string importFile(const std::string &filePath);
    void print(std::string message);
};
