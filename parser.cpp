// parser.cpp
#include "parser.hh"
#include "debugger.hh"
#include "opcodes.hh"
#include "scanner.hh"
#include <fstream>

Parser::Parser(Scanner &scanner)
    : bytecode(Bytecode{})
    , scanner(scanner)
    , nextMemoryLocation(1)
{
    // std::vector<Token> temp = scanner.scanTokens();
    tokens = scanner.scanTokens();
    //    parse();
    while (!isAtEnd()) {
        parse();
    }
}

Bytecode Parser::parse()
{
    scanner.current = 0;
    //Now parse the tokens using scanner methods directly
    std::cout << "======= Scanner Debug =======\n"
              << scanner.toString() << "======= End Debug =======\n"
              << std::endl;
    //   //print("parse function");
    //toString();
    return block();
}

std::string Parser::toString() const
{
    std::cout << "Start Parser Debug" << std::endl;
    std::string result;
    for (const auto &instruction : bytecode)
    {
        result += "Instruction: " + instruction.opcodeToString(instruction.opcode)
                  + " | Line: " + std::to_string(instruction.lineNumber) + "\n";
        //        result += " | Value: " + std::to_string(instruction.value);
        result += " | Value: ";
        //        std::visit(
        //            [&](auto const &val) {
        //                result += std::to_string(val); // Convert to string before adding
        //            },
        //            instruction.value);
        result += "\n";
    }
    std::cout << result << std::endl;
    std::cout << "End Parser Debug" << std::endl;
    return result;
}

void Parser::advance()
{
    if (current < tokens.size()) {
        //  std::cout << current << std::endl;
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

bool Parser::match(TokenType type)
{
    if (peek().type == type)
    {
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

void Parser::error(const std::string &message)
{
    Debugger::error(message,
                    peek().line,
                    current,
                    InterpretationStage::PARSING,
                    scanner.tokenTypeToString(peek().type));
}

Instruction Parser::makeInstruction(Opcode opcode, uint32_t lineNumber)
{
    Instruction instruction(opcode, lineNumber);
    bytecode.push_back(instruction);
    //instruction.debug(); //!Error if this line is uncommented the EOF_TOKEN keeps being parsed indefinitely.
    return instruction;
}

Instruction Parser::makeInstruction(Opcode opcode,
                                    uint32_t lineNumber,
                                    std::variant<int32_t, float, bool, std::string> value)
{
    Instruction instruction(opcode, lineNumber, value);
    bytecode.push_back(instruction);
    //instruction.debug();
    return instruction;
}

Bytecode Parser::block()
{
    //print("block function");
    Bytecode psuedo = statement();
    while (!match(TokenType::RIGHT_BRACE) && !scanner.isAtEnd())
    {
        statement();
        // psuedo.push_back(statement());
    }
    //print("end block function");
    return psuedo;
}

Bytecode Parser::statement()
{
    //print("statement function");
    if (match(TokenType::IF))
    {
        return ifStatement();
    }
    else if (match(TokenType::WHILE))
    {
        return whileStatement();
    }
    else if (match(TokenType::IMPORT))
    {
        return importStatement();
    } else if (match(TokenType::PRINT)) {
        return printStatement();
    } else {
        return expressionStatement();
    }
}

Bytecode Parser::printStatement()
{
    //print("print function");
    Bytecode psuedo;
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'print'.");
    Bytecode expressionResult;
    // Handle different types of expressions after print
    if (peek().type != TokenType::RIGHT_PAREN) {
        expressionResult = expression();
        psuedo.insert(psuedo.end(), expressionResult.begin(), expressionResult.end());
    }
    // Assuming PRINT opcode for printing the value of the expression
    psuedo.push_back(makeInstruction(Opcode::PRINT, peek().line));
    consume(TokenType::RIGHT_PAREN, "Expect ')' after print statement.");
    consume(TokenType::SEMICOLON, "Expected ';' after print statement.");
    //print("end print function");
    return psuedo;
}

Bytecode Parser::importStatement()
{
    ////print("import function");
    consume(TokenType::IMPORT, "Expect 'import' keyword.");
    Token current = peek();
    Token moduleName;
    if (current.type == TokenType::IDENTIFIER)
    {
        moduleName = current;
    }
    else
    {
        error("Expect module name.");
    }
    std::string modulePath = moduleName.lexeme + ".lm";
    std::string sourcecode = importFile(modulePath);
    Scanner importedScanner(sourcecode);
    Parser importedParser(importedScanner);
    Bytecode importedBytecode = importedParser.parse();
    bytecode.insert(bytecode.end(), importedBytecode.begin(), importedBytecode.end());
    return bytecode;
}

std::string Parser::importFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    std::stringstream buffer;

    if (file.is_open())
    {
        buffer << file.rdbuf();
        file.close();
        return buffer.str();
    }
    else
    {
        std::string message = "Unable to open file: " + filePath;
        error(message);
        return "";
    }
}

void Parser::print(std::string message)
{
    Token current = peek();
    std::cout << "Lexeme: " << current.lexeme
              << " token: " << scanner.tokenTypeToString(current.type) << " Message: " << message
              << std::endl
              << std::flush;
    ;
}

Bytecode Parser::ifStatement()
{
    ////print("if function");
    Bytecode psuedo;
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    Bytecode condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
    Bytecode thenBranch = block();
    psuedo.insert(psuedo.end(), condition.begin(), condition.end());
    psuedo.push_back(makeInstruction(Opcode::JUMP_IF_FALSE, thenBranch.front().lineNumber));
    psuedo.insert(psuedo.end(), thenBranch.begin(), thenBranch.end());
    return psuedo;
}

Bytecode Parser::whileStatement()
{
    ////print("while function");
    Bytecode psuedo;
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    Bytecode condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");
    Bytecode body = block(); // Using block() for the while loop's body
    psuedo.insert(psuedo.end(), condition.begin(), condition.end());
    psuedo.push_back(makeInstruction(Opcode::JUMP_IF_FALSE, body.front().lineNumber + 1));
    psuedo.insert(psuedo.end(), body.begin(), body.end());
    psuedo.push_back(makeInstruction(Opcode::JUMP, condition.front().lineNumber));
    return psuedo;
}

Bytecode Parser::expressionStatement()
{
    ////print("expression statement function");
    Bytecode psuedo;
    psuedo = expression();
    // consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return psuedo;
}

Bytecode Parser::expression()
{
    //print("expression function");
    return assignment();
}

Bytecode Parser::assignment()
{
    //print("assignment function");
    Bytecode psuedo = conditional();

    if (match(TokenType::VAR)) {
        varDeclaration();
        // Optionally handle semicolon for statement termination
    } else if (match(TokenType::IDENTIFIER)) {
        varInvoke();
        if (match(TokenType::EQUAL)) {
            Token equals = previous();
            Bytecode right = assignment();

            if (psuedo.size() == 1 && psuedo.back().opcode == Opcode::LOAD_VALUE) {
                // Assignment to a variable
                psuedo.push_back(makeInstruction(Opcode::STORE_VALUE, equals.line));
                psuedo.insert(psuedo.end(), right.begin(), right.end());
            } else {
                // Invalid assignment target
                throw std::runtime_error("Invalid assignment target.");
            }
        }
    }
    //print("end assignment function");
    return psuedo;
}

Bytecode Parser::conditional()
{
    //print("conditional function");
    Bytecode psuedo = logicalOr();

    if (match(TokenType::QUESTION))
    {
        Bytecode thenBranch = expression();
        match(TokenType::COLON);
        Bytecode elseBranch = conditional();

        psuedo.push_back(makeInstruction(Opcode::JUMP_IF_FALSE, thenBranch.front().lineNumber));
        psuedo.insert(psuedo.end(), thenBranch.begin(), thenBranch.end());
        psuedo.push_back(makeInstruction(Opcode::JUMP, elseBranch.front().lineNumber + 1));
        psuedo.insert(psuedo.end(), elseBranch.begin(), elseBranch.end());

        return psuedo;
    }
    //print("end conditional function");
    return psuedo;
}

Bytecode Parser::logicalOr()
{
    //print("or function");
    Bytecode psuedo = logicalAnd();

    while (match(TokenType::OR))
    {
        Token op = previous();
        Bytecode right = logicalAnd();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        psuedo.push_back(makeInstruction(Opcode::OR, op.line));
    }
    //print("end or function");
    return psuedo;
}

Bytecode Parser::logicalAnd()
{
    //print("and function");
    Bytecode psuedo = equality();

    while (match(TokenType::AND))
    {
        Token op = previous();
        Bytecode right = equality();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        psuedo.push_back(makeInstruction(Opcode::AND, op.line));
    }
    //print("end and function");
    return psuedo;
}

//!Todo: Fix this, I am getting wrong values
Bytecode Parser::equality()
{
    //print("equality function");
    Bytecode psuedo = comparison();

    while (match(TokenType::EQUAL_EQUAL) || match(TokenType::BANG_EQUAL))
    {
        Token op = previous();
        Bytecode right = comparison();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        //        psuedo.push_back(
        //            makeInstruction(op.type == TokenType::EQUAL_EQUAL ? Opcode::EQUAL : Opcode::NOT_EQUAL,
        //                            op.line));
        // Generate appropriate instruction based on the operator
        if (op.type == TokenType::EQUAL_EQUAL) {
            psuedo.push_back(makeInstruction(Opcode::EQUAL, op.line));
        } else {
            psuedo.push_back(makeInstruction(Opcode::NOT_EQUAL, op.line));
        }
    }
    //print("end equality function");
    return psuedo;
}

Bytecode Parser::comparison()
{
    //print("camparison function");
    if (isAtEnd()) {
        return primary(); // Empty bytecode for EOF
    }
    Bytecode psuedo = addition();

    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) || match(TokenType::LESS)
           || match(TokenType::LESS_EQUAL)) {
        Token op = previous();
        Bytecode right = addition();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        switch (op.type)
        {
        case TokenType::GREATER:
            psuedo.push_back(makeInstruction(Opcode::GREATER_THAN, op.line));
            break;
        case TokenType::GREATER_EQUAL:
            psuedo.push_back(makeInstruction(Opcode::GREATER_THAN_OR_EQUAL, op.line));
            break;
        case TokenType::LESS:
            psuedo.push_back(makeInstruction(Opcode::LESS_THAN, op.line));
            break;
        case TokenType::LESS_EQUAL:
            psuedo.push_back(makeInstruction(Opcode::LESS_THAN_OR_EQUAL, op.line));
            break;
        default:
            break;
        }
    }
    //print("end comparison function");
    return psuedo;
}

Bytecode Parser::addition()
{
    //print("addition function");
    if (isAtEnd()) {
        return primary(); // Empty bytecode for EOF
    }
    Bytecode psuedo = multiplication();

    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = previous();
        Bytecode right = multiplication();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        psuedo.push_back(makeInstruction(op.type == TokenType::PLUS ? Opcode::ADD : Opcode::SUBTRACT,
                                         op.line,
                                         op.lexeme));
    }
    //print("end addition function");
    return psuedo;
}

Bytecode Parser::multiplication()
{
    //print("multiplication function");
    if (isAtEnd()) {
        return primary(); // Empty bytecode for EOF
    }
    //   //print("mul function");
    Bytecode psuedo = unary();

    while (match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::MODULUS)) {
        //       //print("multiplication function");
        Token op = previous();
        Bytecode right = unary();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        switch (op.type)
        {
        case TokenType::STAR:
            psuedo.push_back(makeInstruction(Opcode::MULTIPLY, op.line, op.lexeme));
            break;
        case TokenType::SLASH:
            psuedo.push_back(makeInstruction(Opcode::DIVIDE, op.line, op.lexeme));
            break;
        case TokenType::MODULUS:
            psuedo.push_back(makeInstruction(Opcode::MODULUS, op.line, op.lexeme));
            break;
        default:
            break;
        }
    }
    //print("end multiplication function");
    return psuedo;
}

Bytecode Parser::unary()
{
    ////print("unary function");
    if (match(TokenType::BANG) || match(TokenType::MINUS))
    {
        Token op = previous();
        Bytecode right = unary();
        right.push_back(makeInstruction(Opcode::NOT,
                                        op.line)); // Assuming NOT opcode represents logical negation
        bytecode.push_back(right.front());
        return right;
    }

    return primary();
}

Bytecode Parser::primary()
{
    //print("primary function");
    Bytecode psuedo;
    Token currentToken = peek();
    switch (currentToken.type) {
    case TokenType::EOF_TOKEN:
        return eof();
    case TokenType::LEFT_PAREN:
        advance();             // Consume '('
        psuedo = expression(); // Parse the expression inside parentheses
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        break;
    case TokenType::RIGHT_PAREN:
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        break;
    case TokenType::LEFT_BRACE:
        block();
        advance();
        break;
    case TokenType::VAR:
        varDeclaration();
        advance();
        break;
    case TokenType::IDENTIFIER:
        if (peekNext().type == TokenType::LEFT_PAREN) {
            // psuedo = functionCall(peek().lexeme);
            std::cout << "Fn call" << std::endl;
        } else {
            std::cout << "Var call" << std::endl;
            varInvoke(); // Handle variable usage
        }
        break;
    case TokenType::STRING:
        psuedo = string();
        advance();
        break;
    case TokenType::NUMBER:
        psuedo = number();
        break;
    case TokenType::TRUE:
        psuedo.push_back(makeInstruction(Opcode::LOAD_CONST, peek().line, true));
        advance();
        break;
    case TokenType::FALSE:
        psuedo.push_back(makeInstruction(Opcode::LOAD_CONST, peek().line, false));
        advance();
        break;
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::MODULUS:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
        // Handle these operators as part of mathematical expressions
        psuedo = expression();
        break;
    case TokenType::IF:
        ifStatement();
        advance();
        break;
    case TokenType::WHILE:
        whileStatement();
        advance();
        break;
    case TokenType::SEMICOLON:
        advance();
        break;
    case TokenType::PRINT:
        printStatement();
        advance();
        break;
        //        error("end of file for primary function");
        //       //print("out of bounds for primary function");
        // add forloops, attempt, parallel, concurency,
    default:
        // Error handling for unexpected tokens
        // statement();
        error("Unexpected token while in primary.");
    }
    //print("end primary function");
    return psuedo;
}

Bytecode Parser::string()
{
    //print("string function");
    Bytecode psuedo;
    Token current = peek();
    std::string stringValue = current.lexeme;
    psuedo.push_back(makeInstruction(Opcode::STORE_STR, peek().line, stringValue));
    //print("end string function");
    return psuedo;
}

Bytecode Parser::number()
{
    //print("number function");
    Bytecode psuedo;
    Token current = peek();
    advance();
    // Convert the numeric literal to a float
    float numberValue = std::stof(current.lexeme);
    psuedo.push_back(makeInstruction(Opcode::LOAD_CONST, peek().line, numberValue));
    //print("end number function");
    return psuedo;
}

Bytecode Parser::eof()
{
    //print("end of file function");
    // Reached the end of the file, stop parsing
    Bytecode psuedo;
    psuedo.push_back(makeInstruction(Opcode::HALT, peek().line));
    //print("exit end of file function");
    //check why we get process crash when we return the
    return psuedo;
}

//Bytecode Parser::block()
//{
//    Bytecode psuedo;
//    advance();             // Consume '{'
//    psuedo = expression(); // Parse the expression inside parentheses
//    consume(TokenType::RIGHT_BRACE, "Expected ')' after expression.");
//    return psuedo;
//}

void Parser::varDeclaration()
{
    // Parse variable declaration syntax
    // Example:
    // var myVar: int = 10;
    //    advance(scanner);         // Consume 'var' token
    //    parseIdentifier(scanner); // Parse variable name
    //    advance(scanner);         // Consume ':' token
    //    parseType(scanner);       // Parse variable type
    //    std::string variableName = scanner.getToken().lexeme;
    advance();
    Token identifierToken = peek(); // Expect identifier token after 'var'
    if (identifierToken.type != TokenType::IDENTIFIER)
    {
        error("Expected identifier after 'var'.");
    }

    // Check if the variable has already been declared
    uint32_t varIndex = symbolTable.getVariableMemoryLocation(identifierToken.lexeme);
    // print(std::to_string(varIndex));
    if (varIndex > 0)
    {
        error("Variable already declared.");
    }
    std::cout << "current location" << nextMemoryLocation << std::endl;
    // Declare the variable in the symbol table with a unique memory location
    uint32_t memoryLocation = nextMemoryLocation++;
    std::cout << "Next location" << nextMemoryLocation << std::endl;
    symbolTable.declareVariable(identifierToken.lexeme, memoryLocation);
    advance(); // Consume identifier token
    if (peekNext().type == TokenType::COLON) {
        advance(); //consume the : token
        advance(); //consume the type token
    }
    if (match(TokenType::EQUAL))
    {
        // Variable assignment
        Bytecode value = expression();
        bytecode.insert(bytecode.end(), value.begin(), value.end());
        // Store the value in the variable's memory location
        Instruction instruct = makeInstruction(Opcode::STORE_VARIABLE,
                                               identifierToken.line,
                                               static_cast<int32_t>(memoryLocation));
        bytecode.push_back(instruct);
    }
    // Optionally handle semicolon for statement termination
    advance();
}

void Parser::varInvoke()
{
    //print("var call function");
    Token identifierToken = peek(); // Expect identifier token after variable name

    if (identifierToken.type != TokenType::IDENTIFIER)
    {
        error("Expected variable name to be an identifier.");
        return;
    }
    print(identifierToken.lexeme);

    // Efficiently look up variable using cached lookup or helper function
    uint32_t memoryLocation = symbolTable.getVariableMemoryLocation(identifierToken.lexeme);

    if (memoryLocation == 0)
    {
        // Handle variable not found (provide clear error message)
        error("Variable '" + identifierToken.lexeme + "' not found in symbol table.");
        return; // Handle potential errors gracefully
    }

    // Generate appropriate bytecode instruction based on memory location
    Instruction instruct = makeInstruction(Opcode::LOAD_VALUE,
                                           identifierToken.line,
                                           static_cast<int32_t>(memoryLocation));
    bytecode.push_back(instruct);

    // Optionally handle semicolons (if applicable)
    consume(TokenType::SEMICOLON, "Variable call should end in semi colon");
}

std::vector<Instruction> Parser::getBytecode() const
{
    std::cout << "Getting the bytecode: " << std::endl;
    // Get the generated bytecode
    return bytecode;
}
