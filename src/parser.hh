//parser.hh
#include "instructions.hh"
#include "parser/algorithm.hh"
#include "parser/packrat.hh"
#include "scanner.hh"
#include "types.hh"
#include "variable.hh"
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
using Packrat = PackratParser;

class Parser
{
public:
    // Constructor
    Parser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem);
    ~Parser() = default;

    // Main parsing function
    Bytecode parse();
    std::string toString() const;                 //debug the parser
    std::vector<Instruction> getBytecode() const; //get the bytecode generated from the parser

private:
    std::vector<Token> tokens;
    bool hadError = false;
    size_t current = 0;                // get the current index position
    std::vector<Instruction> bytecode; // Declare bytecode as a local variable
    bool isNewExpression = true;
    // Scanner instance
    Scanner &scanner;
    PackratParser algo;
    std::unique_ptr<Algorithm> algorithm;

    //variables
    Variables variable; // Instance of Variables class
    std::unordered_map<std::string, int> variableMap; // Use unordered_map to track variable indices
    int variableCounter = 0;                          // Initialize variable counter
    std::shared_ptr<TypeSystem> typeSystem;
    bool isAtEnd() { return tokens[current].type == TokenType::EOF_TOKEN; }
};
