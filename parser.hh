//parser.hh
#include "opcodes.hh"
#include "scanner.hh"
#include <any>
#include <string>
#include <unordered_map>

enum class ReturnType { VOID, INT, FLOAT, BOOL, STRING };
// Define a vector type to hold bytecode instructions
using Bytecode = std::vector<Instruction>;

class SymbolTable
{
public:
    void declareVariable(const std::string &name, uint32_t memoryLocation)
    {
        variables[name] = memoryLocation;
    }

    uint32_t getVariableMemoryLocation(const std::string &name) const
    {
         return variables.count(name) ? variables.at(name) : 0;
        //throw std::runtime_error("Variable not found in symbol table.");
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
    std::vector<Instruction> bytecode; // Declare bytecode as a local variable
    std::vector<Instruction> getBytecode() const;

private:
    Scanner &scanner;
    std::vector<Token> tokens;
    uint32_t nextMemoryLocation;
    SymbolTable symbolTable;

    void advance();
    Token peek();     // Changed to return non-const reference
    Token previous(); // Changed to return non-const reference
    Token peekNext();

    bool match(TokenType type);
    void consume(TokenType type, const std::string &message);
    void error(const std::string &message);

    Instruction makeInstruction(Opcode opcode, uint32_t lineNumber);
    Instruction makeInstruction(Opcode opcode,
                                uint32_t lineNumber,
                                std::variant<int32_t, float, bool, std::string> value);

    Bytecode block();
    Bytecode statement();
    Bytecode importStatement();
    Bytecode ifStatement();
    Bytecode whileStatement();
    Bytecode printStatement();
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
    Bytecode string();
    Bytecode number();
    void varDeclaration();
    void varInvoke();
    std::string importFile(const std::string &filePath);
    void print(std::string message);
};
