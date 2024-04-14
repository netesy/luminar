// parser.cpp
#include "parser.hh"
#include "debugger.hh"
#include "opcodes.hh"
#include "scanner.hh"
#include <fstream>
#include <thread>

Parser::Parser(Scanner &scanner)
    : bytecode(Bytecode{})
    , scanner(scanner)
{
    tokens = scanner.scanTokens();
        parse();
}

Bytecode Parser::parse()
{
    scanner.current = 0;

    while (!isAtEnd()) {
        parseExpression();
    }
    return bytecode;
}

std::string Parser::toString() const
{
    std::string result;
    for (const auto &instruction : bytecode)
    {
        result += "Instruction: " + instruction.opcodeToString(instruction.opcode)
                  + " | Line: " + std::to_string(instruction.lineNumber) + "\n";
        std::string valueStr;
        std::visit(
            [&valueStr](auto const &val) {
                std::stringstream ss;
                ss << val;
                valueStr = ss.str();
            },
            instruction.value);

        result += " | Value: " + valueStr;
        result += "\n";
    }
    //    std::cout << result << std::endl;
    //    std::cout << "End Parser Debug" << std::endl;
    return result;
}

// Pratt parsing utility functions
ParseFn Parser::getParseFn(TokenType type)
{
    //    std::cout << "get parse fn" << std::endl;
    switch (type) {
        // Unary operators
    case TokenType::MINUS:
        //    case TokenType::BANG:
        return &Parser::parseUnary;

        // Binary operators
    case TokenType::PLUS:
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::MODULUS:
        return &Parser::parseBinary;

    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
        return &Parser::parseComparison;

    case TokenType::AND:
    case TokenType::OR:
    case TokenType::BANG:
        return &Parser::parseLogical;

        //Assignment
    case TokenType::PLUS_EQUAL:
    case TokenType::MINUS_EQUAL:
    case TokenType::EQUAL:
        return &Parser::parseAssignment;

        // Literal values
    case TokenType::NUMBER:
    case TokenType::STRING:
    case TokenType::EOF_TOKEN:
        return &Parser::parseLiteral;
    case TokenType::TRUE:
    case TokenType::FALSE:
        return &Parser::parseBoolean;

        // Identifier (variable or function call)
    case TokenType::IDENTIFIER:
        return &Parser::parseVariable;

        // Parentheses (grouping or function call)
    case TokenType::LEFT_PAREN:
        return &Parser::parseParenthesis;

    case TokenType::PRINT:
        return &Parser::parsePrintStatement;
    case TokenType::IF:
        return &Parser::parseIfStatement;
    case TokenType::WHILE:
        return &Parser::parseWhileLoop;
    case TokenType::FOR:
        return &Parser::parseForLoop;
    case TokenType::MATCH:
        return &Parser::parseMatchStatement;

    default:
        return nullptr;
    }
}

void Parser::advance()
{
    if (current < tokens.size()) {
        current++;
    }
}

Token Parser::peek()
{
    return tokens[current];
}

Token Parser::peekNext()
{
    if (current + 1 < tokens.size())
    {
        return tokens[current + 1];
    }
    else
    {
        return Token{TokenType::EOF_TOKEN, "", scanner.getLine()};
    }
}

bool Parser::isAtEnd()
{
    return tokens[current].type == TokenType::EOF_TOKEN;
}

Token Parser::previous()
{
    return tokens[current - 1];
}

bool Parser::check(TokenType type)
{
    if (scanner.isAtEnd())
        return false;
    if (peek().type == type)
    {
        return true;
    }
    return false;
}

bool Parser::match(TokenType type)
{
    if (scanner.isAtEnd())
        return false;
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

void Parser::consume(TokenType type, const std::string &message)
{
    if (!match(type))
    {
        error(message);
    }
}
bool Parser::isExpressionStart(TokenType type)
{
    switch (type) {
        // Literal values
    case TokenType::NUMBER:
    case TokenType::STRING:
    case TokenType::TRUE:
    case TokenType::FALSE:
        return true;
        // Identifiers (variables or function calls)
    case TokenType::IDENTIFIER:
        return true;
        // Parentheses (grouping or function call)
    case TokenType::LEFT_PAREN:
        return true;
        // Binary Unary operators (for completeness)
    case TokenType::MINUS:
    case TokenType::BANG:
    case TokenType::PLUS:
    case TokenType::MODULUS:
    case TokenType::SLASH:
        return true;
    default:
        return false;
    }
}

bool Parser::isBinaryOperator(TokenType type)
{
    switch (type) {
        // Binary values
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::SLASH:
    case TokenType::STAR:
    case TokenType::MODULUS:
        return true;
    default:
        return false;
    }
}

void Parser::error(const std::string &message)
{
    Debugger::error(message,
                    peek().line,
                    current,
                    InterpretationStage::PARSING,
                    scanner.tokenTypeToString(peek().type));
}

void Parser::parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixParseFn = getParseFn(peek().type);

    if (prefixParseFn == nullptr) {
        error("Unexpected token");
        return;
    }

    bool shouldParseInfix = true;
    (this->*prefixParseFn)();

    while (precedence <= getTokenPrecedence(peekNext().type)) {
        if (isAtEnd()) {
            break;
        }

        advance();
        ParseFn infixParseFn = getParseFn(peek().type);

        if (infixParseFn == nullptr) {
            error("Unexpected token");
            return;
        }

        (this->*infixParseFn)();
    }
}

Precedence Parser::getTokenPrecedence(TokenType type)
{
    switch (type) {
        // Logical OR
    case TokenType::OR:
        return PREC_OR;
        // Logical AND
    case TokenType::AND:
        return PREC_AND;
        // Comparison operators
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
        return PREC_EQUALITY;
        // Addition and subtraction
    case TokenType::PLUS:
    case TokenType::MINUS:
        return PREC_TERM;
        // Multiplication and division
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::MODULUS:
        return PREC_FACTOR;
        // Unary operators
    case TokenType::BANG:
        return PREC_UNARY;
        // Function or method calls
    case TokenType::LEFT_PAREN:
    case TokenType::DOT:
        return PREC_CALL;
        // Highest precedence
    case TokenType::NUMBER:
    case TokenType::STRING:
    case TokenType::IDENTIFIER:
    case TokenType::TRUE:
    case TokenType::FALSE:
        return PREC_PRIMARY;
    default:
        // Lowest precedence
        return PREC_NONE;
    }
}

Instruction Parser::makeInstruction(Opcode opcode, uint32_t lineNumber)
{
    Instruction instruction(opcode, lineNumber);
    bytecode.push_back(instruction);
    // instruction.debug(); //use this to debug the instruction generation
    return instruction;
}

Instruction Parser::makeInstruction(Opcode opcode,
                                    uint32_t lineNumber,
                                    std::variant<int32_t, double, bool, std::string> value)
{
    Instruction instruction(opcode, lineNumber, value);
    bytecode.push_back(instruction);
    //  instruction.debug(); //use this to debug the instruction generation
    return instruction;
}

void Parser::parsePrimary()
{
    if (isAtEnd()) {
        return; // Stop parsing if EOF_TOKEN is reached
    }

    ParseFn prefixParseFn = getParseFn(peek().type);
    if (prefixParseFn != nullptr) {
        (this->*prefixParseFn)();
    } else {
        error("Unexpected token in primary expressions");
        return;
    }

    // Parse binary operations with higher precedence
    while (!isAtEnd()) {
        TokenType nextOp = peek().type;
        Precedence nextPrecedence = getTokenPrecedence(nextOp);

        // Stop parsing if the next token has lower or equal precedence
        if (nextPrecedence <= getTokenPrecedence(peek().type)) {
            break;
        }

        // Parse the next token with higher precedence
        ParseFn infixParseFn = getParseFn(nextOp);
        if (infixParseFn != nullptr) {
            (this->*infixParseFn)();
        } else {
            error("Unexpected token in expression primary");
            return;
        }
    }
}

void Parser::parseExpression()
{

    parsePrimary();

    while (!isAtEnd() && isBinaryOperator(peek().type)) {
        TokenType operatorType = peek().type;
        advance(); // Move past the operator

        // Parse the right-hand side operand
        parsePrimary();

        // Generate bytecode for the binary operation
        switch (operatorType) {
        case TokenType::PLUS:
            makeInstruction(Opcode::ADD, peek().line);
            break;
        case TokenType::MINUS:
            makeInstruction(Opcode::SUBTRACT, peek().line);
            break;
        case TokenType::STAR:
            makeInstruction(Opcode::MULTIPLY, peek().line);
            break;
        case TokenType::SLASH:
            makeInstruction(Opcode::DIVIDE, peek().line);
            break;
        case TokenType::MODULUS:
            makeInstruction(Opcode::MODULUS, peek().line);
            break;
        default:
            error("Unexpected binary operator");
            break;
        }
    }
}

void Parser::parseVariable()
{
    //    std::cout << "get variable fn" << std::endl;
    if (match(TokenType::IDENTIFIER)) {
        std::string value = previous().lexeme;
        consume(TokenType::IDENTIFIER, "Expected variable name");
        // Add the variable to the symbol table and get its memory location
        uint32_t memoryLocation = symbolTable.addVariable(value);

        // Generate bytecode for loading the variable's value
        bytecode.push_back(makeInstruction(Opcode::LOAD_VARIABLE,
                                           peek().line,
                                           std::any_cast<int32_t>(memoryLocation)));
    } else {
        error("Unexpected token in variable expression");
    }
}

// Parse assignment expression
void Parser::parseAssignment()
{
    //    std::cout << "get assignment fn" << std::endl;
    parseVariable(); // Parse the variable being assigned to
    TokenType toke;
    if (match(TokenType::EQUAL)) {
        toke = TokenType::EQUAL;
    } else if (match(TokenType::PLUS_EQUAL)) {
        toke = TokenType::PLUS_EQUAL;
    } else if (match(TokenType::MINUS_EQUAL)) {
        toke = TokenType::MINUS_EQUAL;
    } else {
        // Handle error: no valid assignment operator found
        error("Expected assignment operator (=, +=, -=)");
        toke = TokenType::UNDEFINED;
    }

    parseExpression(); // Parse the expression for the new value

    // Generate bytecode for assignment based on the operator
    switch (toke) {
    case TokenType::EQUAL:
        makeInstruction(Opcode::STORE_VARIABLE, peek().line);
        break;
    case TokenType::PLUS_EQUAL:
        makeInstruction(Opcode::ADD_ASSIGN, peek().line);
        break;
    case TokenType::MINUS_EQUAL:
        makeInstruction(Opcode::SUB_ASSIGN, peek().line);
        break;
    default:
        // Should not happen, but handle unexpected operator for safety
        error("Internal error: unexpected assignment operator");
    }
}

void Parser::parseBinary()
{
    // std::cout << "get binary fn" << std::endl;
    TokenType operatorType = previous().type; // Get the operator type

    parsePrecedence(getTokenPrecedence(operatorType));

    // Generate bytecode for the binary operation
    switch (operatorType) {
    case TokenType::PLUS:
        makeInstruction(Opcode::ADD, peek().line);
        break;
    case TokenType::MINUS:
        makeInstruction(Opcode::SUBTRACT, peek().line);
        break;
    case TokenType::STAR:
        makeInstruction(Opcode::MULTIPLY, peek().line);
        break;
    case TokenType::SLASH:
        makeInstruction(Opcode::DIVIDE, peek().line);
        break;
    case TokenType::MODULUS:
        makeInstruction(Opcode::MODULUS, peek().line);
        break;
    default:
        error("Expected binary operator (+, -, *, /, %)");
        break;
    }
}

void Parser::parseLogical()
{
    // std::cout << "get logical fn" << std::endl;
    TokenType operatorType = peek().type; // Get the operator type
    //    std::cout << "token: " << peek().lexeme << std::endl;

    parsePrecedence(getTokenPrecedence(operatorType));

    // Generate bytecode for the logical operation
    switch (operatorType) {
    case TokenType::BANG:
        makeInstruction(Opcode::NOT, peek().line);
        break;
    case TokenType::AND:
        makeInstruction(Opcode::AND, peek().line);
        break;
    case TokenType::OR:
        makeInstruction(Opcode::OR, peek().line);
        break;
    default:
        error("Expected logical operator (!, &, |)");
        break;
    }
}

void Parser::parseComparison()
{
    // std::cout << "get comparison fn" << std::endl;
    TokenType operatorType = peek().type; // Get the operator type

    // Parse additional comparison expressions with the same or higher precedence

    parsePrecedence(getTokenPrecedence(operatorType));

    // Generate bytecode for the comparison operation
    switch (operatorType) {
    case TokenType::GREATER:
        makeInstruction(Opcode::GREATER_THAN, peek().line);
        break;
    case TokenType::GREATER_EQUAL:
        makeInstruction(Opcode::GREATER_THAN_OR_EQUAL, peek().line);
        break;
    case TokenType::LESS:
        makeInstruction(Opcode::LESS_THAN, peek().line);
        break;
    case TokenType::LESS_EQUAL:
        makeInstruction(Opcode::LESS_THAN_OR_EQUAL, peek().line);
        break;
    case TokenType::EQUAL_EQUAL:
        makeInstruction(Opcode::EQUAL, peek().line);
        break;
    case TokenType::BANG_EQUAL:
        makeInstruction(Opcode::NOT_EQUAL, peek().line);
        break;
    default:
        error("Expected comparison operator (>, >=, <, <=, ==, !=)");
        break;
    }
}

void Parser::parseUnary()
{
    if (match(TokenType::MINUS) || match(TokenType::BANG)) {
        TokenType op = previous().type;
        parseUnary(); // Recursively parse unary operators

        // Check if the next token is a literal or another unary operator
        if (!isAtEnd() && isExpressionStart(peek().type)) {
            // Generate bytecode for the unary operator
            switch (op) {
            case TokenType::MINUS:
                makeInstruction(Opcode::NEGATE, peek().line);
                break;
            case TokenType::BANG:
                makeInstruction(Opcode::NOT, peek().line);
                break;
            default:
                // Handle other unary operators if needed
                break;
            }
        } else {
            error("Unexpected token after unary operator");
            return;
        }
    } else {
        // If it's not a unary operator, continue with regular parsing
        parsePrimary();
    }
}

void Parser::parseBoolean()
{
    //    std::cout << "get boolean fn" << std::endl;
    if (match(TokenType::TRUE)) {
        makeInstruction(Opcode::BOOLEAN, peek().line, true);
    } else if (match(TokenType::FALSE)) {
        makeInstruction(Opcode::BOOLEAN, peek().line, false);
    } else {
        error("Expected boolean operator (true, false, 1, 0)");
    }
}

void Parser::parseLiteral()
{
    // std::cout << "get literal fn" << std::endl;
    Token op = peek();
    if (match(TokenType::NUMBER)) {
        double value = std::stod(previous().lexeme);
        makeInstruction(Opcode::LOAD_CONST, op.line, value);
    } else if (match(TokenType::STRING)) {
        std::string value = previous().lexeme;
        makeInstruction(Opcode::LOAD_STR, op.line, value);
    } else if (match(TokenType::EOF_TOKEN)) {
        std::cout << "end of token" << std::endl;
        return;
        // makeInstruction(Opcode::HALT, op.line);
    } else {
        error("Unexpected token in literal expression");
    }
}

// Parse statements (needs adaptation from first parser)
void Parser::parseStatement()
{
    //    std::cout << "get statement fn" << std::endl;
    if (isExpressionStart(previous().type)) {
        parseExpression();
    } else if (check(TokenType::LEFT_BRACE)) {
        parseBlock();
    } else if (check(TokenType::LEFT_PAREN)) {
        parseParenthesis();
    } else if (check(TokenType::PRINT)) {
        parsePrintStatement();
    } else if (check(TokenType::IF)) {
        parseIfStatement();
    } else if (check(TokenType::WHILE)) {
        parseWhileLoop();
    } else if (check(TokenType::FOR)) {
        parseForLoop();
    } else if (check(TokenType::MATCH)) {
        parseMatchStatement();
    } else {
        error("Unexpected token at start of statement");
    }
}

// Parse code blocks
void Parser::parseBlock()
{
    consume(TokenType::LEFT_BRACE, "Expected '{' at start of block.");
    while (!match(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        parseStatement();
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' at end of block.");
}

//parse grouping
void Parser::parseParenthesis()
{
    //    std::cout << "parentheses" << std::endl;
    consume(TokenType::LEFT_PAREN, "Expected '(' at start of group.");
    while (!check(TokenType::RIGHT_PAREN) && !isAtEnd()) {
        parseExpression();
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' at end of group.");
}

// Pratt parsing functions (adapt from first parser or rewrite)

// Parse print statements
void Parser::parsePrintStatement()
{
    consume(TokenType::PRINT, "Expected 'print' keyword.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'print'.");

    //Todo How can we save the parsed expression to the print command?
    parseStatement();

    // Generate bytecode for printing the expression's value (implementation needed)
    // You'll need to consider the data type of the expression and VM's printing functionality.
    makeInstruction(Opcode::PRINT, peek().line);

    consume(TokenType::RIGHT_PAREN, "Expected ')' after expression in print statement.");
}

// Parse if statements
void Parser::parseIfStatement()
{
    consume(TokenType::IF, "Expected 'if' keyword.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'.");

    parseExpression();

    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'if' statement.");

    // Generate bytecode for conditional jump based on the condition (implementation needed)
    // You might need temporary labels for the then and else blocks.
    makeInstruction(Opcode::JUMP_IF_FALSE, peek().line,
                    0); // Placeholder for jump address

    parseStatement(); // Parse the then block

    if (match(TokenType::ELSE)) {
        Bytecode elseBlock;
        // Generate bytecode to jump over the then block (implementation needed)
        makeInstruction(Opcode::JUMP, peek().line,
                        0); // Placeholder for jump address
        parseStatement();   // Parse the else block
        elseBlock.push_back(bytecode.back());
        bytecode.pop_back(); // Remove the jump instruction before the else block
        bytecode.insert(bytecode.end(), elseBlock.begin(), elseBlock.end());
    }

    // Patch the jump instruction addresses (implementation needed)
    // You'll need to update the jump instructions based on the actual bytecode positions.
}

// Parse while loops (basic structure)
void Parser::parseWhileLoop()
{
    consume(TokenType::WHILE, "Expected 'while' keyword.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'.");

    parseExpression();

    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'while' loop.");

    // Generate bytecode for loop header (implementation needed)
    // You might need a label for the loop start and jump instruction.
    bytecode.push_back(makeInstruction(Opcode::WHILE_LOOP, //Opcode::LABEL
                                       peek().line,
                                       0)); // Placeholder for loop start label

    parseStatement(); // Parse the loop body

    // Generate bytecode for jump back to the loop condition (implementation needed)
    makeInstruction(Opcode::JUMP, peek().line,
                    0); // Placeholder for jump address

    // Patch the jump instruction addresses (implementation needed)
    // You'll need to update the jump instructions based on the actual bytecode positions.
}

// Parse for loops (basic structure, Python-like for loop not directly implemented)
void Parser::parseForLoop()
{
    consume(TokenType::FOR, "Expected 'for' keyword.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'.");

    // Implement parsing of loop variable initialization, condition, and increment/decrement (omitted here for simplicity)
    // You'll need to parse these elements and potentially generate bytecode for initialization and increment/decrement.

    consume(TokenType::RIGHT_PAREN, "Expected ')' after loop initialization in 'for' loop.");

    parseStatement(); // Parse the loop body

    // Implement bytecode for loop control (omitted here for simplicity)
    // You might need a loop counter and jump instructions based on the condition.
}

// Parse match statements (not directly implemented here)
void Parser::parseMatchStatement()
{
    error("Match statements not currently supported.");
}

std::vector<Instruction> Parser::getBytecode() const
{
    std::cout << "Getting the bytecode: " << std::endl;
    // Get the generated bytecode
    return bytecode;
}
