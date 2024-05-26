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
    case TokenType::AND:
    return &Parser::parseAnd;
    case TokenType::OR:
    return &Parser::parseOr;
    case TokenType::BANG:
        return &Parser::parseLogical;

    case TokenType::PLUS:
    case TokenType::STAR:
        return &Parser::parseBinary;
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

        //Assignment
    case TokenType::PLUS_EQUAL:
    case TokenType::MINUS_EQUAL:
    case TokenType::EQUAL:
        return &Parser::parseAssignment;

        // Literal values
    case TokenType::NUMBER:
    case TokenType::STRING:
        return &Parser::parseLiteral;
    case TokenType::EOF_TOKEN:
        return &Parser::parseEOF;
    case TokenType::TRUE:
    case TokenType::FALSE:
        return &Parser::parseBoolean;

        // Identifier (variable or function call)
    case TokenType::VAR:
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
bool Parser::isExpression(TokenType type)
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
    //Comparison Operators
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
        return true;
        // Logical operators (and, or)
    // case TokenType::AND:
    // case TokenType::OR:
    //     return true;
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

    (this->*prefixParseFn)();


    while (precedence >= getTokenPrecedence(peekNext().type)) {
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



// void Parser::parsePrecedence(Precedence precedence)
// {
//     advance();
//     ParseFn prefixParseFn = getParseFn(peek().type);

//     if (prefixParseFn == nullptr) {
//         error("Unexpected token");
//         return;
//     }

//     bool shouldParseInfix = true;
//     (this->*prefixParseFn)();

//     while (precedence <= getTokenPrecedence(peekNext().type)) {
//         if (isAtEnd()) {
//             break;
//         }

//         TokenType nextOp = peekNext().type;
//         Precedence nextPrecedence = getTokenPrecedence(nextOp);

//         //Handle the case where a logical operator is followed by a comparison operator
//         if (precedence == PREC_OR || precedence == PREC_AND) {
//             if (nextPrecedence == PREC_EQUALITY) {
//                 // Parse the comparison expression first
//                 advance();
//                 ParseFn comparisonParseFn = getParseFn(peek().type);
//                 (this->*comparisonParseFn)();

//                 // Then apply the logical operator
//                 ParseFn logicalParseFn = getParseFn(nextOp);
//                 (this->*logicalParseFn)();

//                 continue;
//             }
//         }

//         advance();
//         ParseFn infixParseFn = getParseFn(peek().type);

//         if (infixParseFn == nullptr) {
//             error("Unexpected token");
//             return;
//         }

//         (this->*infixParseFn)();
//     }
// }

void Parser::parseEOF()
{
    Token op = peek();
    if (match(TokenType::EOF_TOKEN)){
        std::cout << "Unexpected end of token" << std::endl;
        emit(Opcode::HALT, op.line);
        return;
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

Instruction Parser::emit(Opcode opcode, uint32_t lineNumber)
{
    Instruction instruction(opcode, lineNumber);
    bytecode.push_back(instruction);
    // instruction.debug(); //use this to debug the instruction generation
    return instruction;
}

Instruction Parser::emit(Opcode opcode,
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
    parseEOF();

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

    while (!isAtEnd() && isExpression(peek().type)) {
        TokenType operatorType = peek().type;
        advance(); // Move past the operator 

        // Parse the right-hand side operand
        parsePrimary();

        // Generate bytecode for the binary operation
        switch (operatorType)
        {
        case TokenType::PLUS:
            emit(Opcode::ADD, peek().line);
            break;
        case TokenType::MINUS:
            emit(Opcode::SUBTRACT, peek().line);
            break;
        case TokenType::STAR: // The '*' operator is not handled here
            emit(Opcode::MULTIPLY, peek().line);
            break;
        case TokenType::SLASH:
            emit(Opcode::DIVIDE, peek().line);
            break;
        case TokenType::MODULUS:
            emit(Opcode::MODULUS, peek().line);
            break;
        case TokenType::GREATER:
            emit(Opcode::GREATER_THAN, peek().line);
            break;
        case TokenType::GREATER_EQUAL:
            emit(Opcode::GREATER_THAN_OR_EQUAL, peek().line);
            break;
        case TokenType::LESS:
            emit(Opcode::LESS_THAN, peek().line);
            break;
        case TokenType::LESS_EQUAL:
            emit(Opcode::LESS_THAN_OR_EQUAL, peek().line);
            break;
        case TokenType::EQUAL_EQUAL:
            emit(Opcode::EQUAL, peek().line);
            break;
        case TokenType::BANG_EQUAL:
            emit(Opcode::NOT_EQUAL, peek().line);
            break;
        default:
            error("Unexpected binary operator");
            break;
        }
    }
}

void Parser::parseVariable()
{
    std::cout << "get variable fn" << std::endl;
    if (match(TokenType::IDENTIFIER)) {
        std::string value = previous().lexeme;
        consume(TokenType::IDENTIFIER, "Expected variable name");
        // Add the variable to the symbol table and get its memory location
        uint32_t memoryLocation = symbolTable.addVariable(value);

        // Generate bytecode for loading the variable's value
        bytecode.push_back(emit(Opcode::LOAD_VARIABLE,
                            peek().line,
                            std::any_cast<int32_t>(memoryLocation)));
    } else if (match(TokenType::VAR)) {
        //implement the var declaration
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
        emit(Opcode::STORE_VARIABLE, peek().line);
        break;
    case TokenType::PLUS_EQUAL:
        emit(Opcode::ADD_ASSIGN, peek().line);
        break;
    case TokenType::MINUS_EQUAL:
        emit(Opcode::SUB_ASSIGN, peek().line);
        break;
    default:
        // Should not happen, but handle unexpected operator for safety
        error("Internal error: unexpected assignment operator");
    }
}

void Parser::parseBinary()
{
    parseEOF();
    // std::cout << "get binary fn" << std::endl;
    TokenType operatorType = previous().type; // Get the operator type

    parsePrecedence(getTokenPrecedence(operatorType));

    // Generate bytecode for the binary operation
    switch (operatorType) {
    case TokenType::PLUS:
        emit(Opcode::ADD, peek().line);
        break;
    case TokenType::MINUS:
        emit(Opcode::SUBTRACT, peek().line);
        break;
    case TokenType::STAR:
        emit(Opcode::MULTIPLY, peek().line);
        break;
    case TokenType::SLASH:
        emit(Opcode::DIVIDE, peek().line);
        break;
    case TokenType::MODULUS:
        emit(Opcode::MODULUS, peek().line);
        break;
    default:
        error("Expected binary operator (+, -, *, /, %)");
        break;
    }
}

void Parser::parseAnd()
{
    std::cout << "get and fn" << std::endl;
    TokenType operatorType = peek().type; // Get the operator type
    parsePrecedence(PREC_AND); // Start parsing with the AND precedence

    while (!isAtEnd() && operatorType == TokenType::AND) {
        advance(); // Advance past the 'and' token
        parsePrecedence(PREC_COMPARISON); // Parse the right-hand side operand

    }
        if(operatorType == TokenType::AND){
        std::cout << "show and fn" << std::endl;
        emit(Opcode::AND, peek().line);
    }
}

void Parser::parseOr()
{
    std::cout << "get or fn" << std::endl;
    TokenType operatorType = peek().type; // Get the operator type
    parsePrecedence(PREC_OR); // Start parsing with the OR precedence

    while (!isAtEnd() && operatorType == TokenType::OR) {
        advance(); // Advance past the 'or' token

        parsePrecedence(PREC_OR); // Parse the right-hand side operand

        // emit(Opcode::OR, operatorToken.line);
    }
    if(operatorType == TokenType::OR){
        std::cout << "show or fn" << std::endl;
        emit(Opcode::OR, peek().line);
    }
}

void Parser::parseLogical()
{
    std::cout << "get logical fn" << std::endl;
    TokenType operatorType = peek().type; // Get the operator type
    std::cout << "logical token: " << peek().lexeme << std::endl;
    //parsePrecedence(getTokenPrecedence(operatorType));
    std::cout << "2nd token: " << peek().lexeme << std::endl;
    if (operatorType == TokenType::BANG) {
        advance();
        parsePrecedence(PREC_UNARY);
        emit(Opcode::NOT, peek().line);
    } else {
        parseOr();
    }
}

void Parser::parseComparison()
{
    parseEOF();
    std::cout << "get comparison fn" << std::endl;
    TokenType operatorType = peek().type; // Get the operator type
    // Parse additional comparison expressions with the same or higher precedence
    parsePrecedence(getTokenPrecedence(operatorType));

    // Generate bytecode for the comparison operation
    switch (operatorType) {
    case TokenType::GREATER:
        emit(Opcode::GREATER_THAN, peek().line);
        break;
    case TokenType::GREATER_EQUAL:
        emit(Opcode::GREATER_THAN_OR_EQUAL, peek().line);
        break;
    case TokenType::LESS:
        emit(Opcode::LESS_THAN, peek().line);
        break;
    case TokenType::LESS_EQUAL:
        emit(Opcode::LESS_THAN_OR_EQUAL, peek().line);
        break;
    case TokenType::EQUAL_EQUAL:
        emit(Opcode::EQUAL, peek().line);
        break;
    case TokenType::BANG_EQUAL:
        emit(Opcode::NOT_EQUAL, peek().line);
        break;
    default:
       // error("Expected comparison operator (>, >=, <, <=, ==, !=)");
        break;
    }
}

void Parser::parseUnary()
{
    if (match(TokenType::MINUS)) {
        // Check if the unary minus is followed by a number
        if (check(TokenType::NUMBER)) {
            // If it's followed by a number, parse it as a literal and negate its value
            parseLiteral();
            emit(Opcode::NEGATE, peek().line);
        } else {
            // If it's not followed by a number, parse the next expression recursively
            parseUnary(); // Recursively parse unary operators

            // Generate bytecode for the unary operator
            emit(Opcode::NEGATE, peek().line);
        }
    } else if (match(TokenType::BANG)) {
        // Handle logical NOT operator
        parseUnary(); // Recursively parse unary operators

        // Generate bytecode for the logical NOT operator
        emit(Opcode::NOT, peek().line);
    } else {
        // If it's not a unary operator, continue with regular parsing
        parsePrimary();
    }
}

void Parser::parseBoolean()
{
    //    std::cout << "get boolean fn" << std::endl;
    if (match(TokenType::TRUE)) {
        emit(Opcode::BOOLEAN, peek().line, true);
    } else if (match(TokenType::FALSE)) {
        emit(Opcode::BOOLEAN, peek().line, false);
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
        emit(Opcode::LOAD_CONST, op.line, value);
    } else if (match(TokenType::STRING)) {
        std::string value = previous().lexeme;
        emit(Opcode::LOAD_STR, op.line, value);
    } else {
        error("Unexpected token in literal expression");
    }
}

// Parse statements (needs adaptation from first parser)
void Parser::parseStatement()
{
    //    std::cout << "get statement fn" << std::endl;
    if (isExpression(previous().type)) {
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
    std::cout << "parentheses" << std::endl;
    consume(TokenType::LEFT_PAREN, "Expected '(' at start of group.");
    while (!check(TokenType::RIGHT_PAREN) && !isAtEnd()) {
        parseExpression();
    }

    if (isAtEnd()) {
        return; // Stop parsing if EOF_TOKEN is reached
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
    emit(Opcode::PRINT, peek().line);

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
    emit(Opcode::JUMP_IF_FALSE, peek().line,
                    0); // Placeholder for jump address

    parseStatement(); // Parse the then block

    if (match(TokenType::ELSE)) {
        Bytecode elseBlock;
        // Generate bytecode to jump over the then block (implementation needed)
        emit(Opcode::JUMP, peek().line,
                        0); // Placeholder for jump address
        parseStatement();   // Parse the else block
        elseBlock.push_back(bytecode.back());
        bytecode.pop_back(); // Remove the jump instruction before the else block
        bytecode.insert(bytecode.end(), elseBlock.begin(), elseBlock.end());
    }

    // Patch the jump instruction addresses (implementation needed)
    // You'll need to update the jump instructions based on the actual bytecode positions.
}

// void Parser::parseIfStatement() {
//     consume(TokenType::IF, "Expected 'if' keyword.");
//     parseExpression();
//     consume(TokenType::THEN, "Expected 'then' keyword.");
//     parseStatement(); // Parse the if body recursively
//     if (check(TokenType::ELSE)) {
//         consume(TokenType::ELSE, "Expected 'else' keyword.");
//         parseStatement(); // Parse the else body recursively
//     }
// }

// Parse while loops (basic structure)
void Parser::parseWhileLoop()
{
    consume(TokenType::WHILE, "Expected 'while' keyword.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'.");

    parseExpression();

    consume(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'while' loop.");

    // Generate bytecode for loop header (implementation needed)
    // You might need a label for the loop start and jump instruction.
    bytecode.push_back(emit(Opcode::WHILE_LOOP, //Opcode::LABEL
                                       peek().line,
                                       0)); // Placeholder for loop start label

    parseStatement(); // Parse the loop body

    // Generate bytecode for jump back to the loop condition (implementation needed)
    emit(Opcode::JUMP, peek().line,
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
    //implement parsing of match statements
}

std::vector<Instruction> Parser::getBytecode() const
{
    std::cout << "Getting the bytecode: " << std::endl;
    // Get the generated bytecode
    return bytecode;
}
