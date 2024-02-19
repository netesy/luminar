//parser.cpp
#include "parser.hh"
#include "debugger.hh"
#include "opcodes.hh"
#include "scanner.hh"
#include <fstream>

Parser::Parser(Scanner &scanner)
    : scanner(scanner)
    , nextMemoryLocation(1)
    , bytecode(Bytecode{})
{
    // std::vector<Token> temp = scanner.scanTokens();
    tokens = scanner.scanTokens();
}

Bytecode Parser::parse()
{
    scanner.current = 0;
    // // Now parse the tokens using scanner methods directly
    // std::cout << "======= Scanner Debug =======\n"
    //           << scanner.toString() << "======= End Debug =======\n"
    //           << std::endl;
    //print("parse function");
    return block();
}

std::string Parser::toString() const
{
    std::cout << "Start Parser Debug" << std::endl;
    std::string result;
    for (const auto &instruction : bytecode) {
        result += "Instruction: " + instruction.opcodeToString(instruction.opcode)
                  + " | Line: " + std::to_string(instruction.lineNumber) + "\n";
        if (instruction.boolValue)
            result += " | Value: " + std::to_string(instruction.boolValue);
        if (instruction.floatValue)
            result += " | Value: " + std::to_string(instruction.floatValue);
        if (instruction.intValue)
            result += " | Value: " + std::to_string(instruction.intValue);
        if (instruction.stringValue != "")
            result += "| Value: " + instruction.stringValue;
    }
    std::cout << result << std::endl;
    std::cout << "End Parser Debug" << std::endl;
    return result;
}

uint32_t Parser::getFunctionMemoryLocation(const std::string &name) const
{
    if (functionTable.count(name)) {
        return functionTable.at(name).memoryLocation;
    }
    throw std::runtime_error("Function not found in symbol table.");
}
void Parser::advance()
{
    if (current < tokens.size()) {
        current++;
       // std::cout << "current index: " << current <<  " of token size: " << tokens.size() << std::endl;
    }
    // scanner.advance();
}

Token Parser::peek()
{
    return tokens[current];
    //    return scanner.getToken();
}

Token Parser::peekNext()
{
    if (current + 1 < tokens.size()) {
        return tokens[current + 1];
    } else {
        return Token{TokenType::EOF_TOKEN, "", scanner.getLine()};
    }

    //    return scanner.getNextToken();
}

Token Parser::previous()
{
    return tokens[current - 1];
    //    return scanner.getPrevToken();
}

bool Parser::match(TokenType type)
{
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

void Parser::consume(TokenType type, const std::string &message)
{
    if (!match(type)) {
        error(message);
    }
}

void Parser::error(const std::string &message)
{
    Debugger::error(message, peek().line, 0, InterpretationStage::PARSING, peek().lexeme);
}

Instruction Parser::makeInstruction(Opcode opcode, uint32_t lineNumber)
{
    Instruction instruction(opcode, lineNumber);
    bytecode.push_back(instruction);
    instruction.debug();
    return instruction;
}

Instruction Parser::makeInstruction(Opcode opcode, uint32_t lineNumber, float floatValue)
{
    Instruction instruction(opcode, lineNumber, floatValue);
    bytecode.push_back(instruction);
    instruction.debug();
    return instruction;
}

Instruction Parser::makeInstruction(Opcode opcode, uint32_t lineNumber, std::string value)
{
    Instruction instruction(opcode, lineNumber, value);
    bytecode.push_back(instruction);
    instruction.debug();
    return instruction;
}

Instruction Parser::makeInstruction(Opcode opcode, uint32_t lineNumber, int32_t intValue)
{
    Instruction instruction(opcode, lineNumber, intValue);
    bytecode.push_back(instruction);
    instruction.debug();
    return instruction;
}

Instruction Parser::makeInstruction(Opcode opcode, uint32_t lineNumber, bool boolValue)
{
    Instruction instruction(opcode, lineNumber, boolValue);
    bytecode.push_back(instruction);
    instruction.debug();
    return instruction;
}

Bytecode Parser::block()
{
    //print("block function");
    Bytecode psuedo = statement();
    while (!match(TokenType::RIGHT_BRACE) && !scanner.isAtEnd()) {
        statement();
        //            //            psuedo.push_back(statement());
    }
    return psuedo;
}

Bytecode Parser::statement()
{
    //print("statement function");
    if (match(TokenType::IF)) {
        return ifStatement();
    } else if (match(TokenType::WHILE)) {
        return whileStatement();
    } else {
        return expressionStatement();
    }
}

Bytecode Parser::importStatement()
{
    //print("import function");
    consume(TokenType::IMPORT, "Expect 'import' keyword.");
    Token current = peek();
    Token moduleName;
    if (current.type == TokenType::IDENTIFIER) {
        moduleName = current;
    } else {
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

    if (file.is_open()) {
        buffer << file.rdbuf();
        file.close();
        return buffer.str();
    } else {
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
              << std::endl<< std::flush;;
}

Bytecode Parser::ifStatement()
{
    //print("if function");
    Bytecode psuedo;
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    Bytecode condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
    Bytecode thenBranch = statement();
    psuedo.insert(psuedo.end(), condition.begin(), condition.end());
    psuedo.push_back(makeInstruction(Opcode::JUMP_IF_FALSE, thenBranch.front().lineNumber));
    psuedo.insert(psuedo.end(), thenBranch.begin(), thenBranch.end());
    return psuedo;
}

Bytecode Parser::whileStatement()
{
    //print("while function");
    Bytecode psuedo;
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    Bytecode condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");
    Bytecode body = statement();
    psuedo.insert(psuedo.end(), condition.begin(), condition.end());
    psuedo.push_back(makeInstruction(Opcode::JUMP_IF_FALSE, body.front().lineNumber + 1));
    psuedo.insert(psuedo.end(), body.begin(), body.end());
    psuedo.push_back(makeInstruction(Opcode::JUMP, condition.front().lineNumber));
    return psuedo;
}

Bytecode Parser::expressionStatement()
{
    //print("expression statement function");
    Bytecode psuedo;
    psuedo = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return psuedo;
}

Bytecode Parser::expression()
{
    //print("expression function");
    return assignment();
}

Bytecode Parser::conditional()
{
    //print("if conditional function");
    Bytecode psuedo = logicalOr();

    if (match(TokenType::QUESTION)) {
        Bytecode thenBranch = expression();
        match(TokenType::COLON);
        Bytecode elseBranch = conditional();

        psuedo.push_back(makeInstruction(Opcode::JUMP_IF_FALSE, thenBranch.front().lineNumber));
        psuedo.insert(psuedo.end(), thenBranch.begin(), thenBranch.end());
        psuedo.push_back(makeInstruction(Opcode::JUMP, elseBranch.front().lineNumber + 1));
        psuedo.insert(psuedo.end(), elseBranch.begin(), elseBranch.end());

        return psuedo;
    }

    return psuedo;
}

Bytecode Parser::logicalOr()
{
    //print("or function");
    Bytecode psuedo = logicalAnd();

    while (match(TokenType::OR)) {
        Token op = previous();
        Bytecode right = logicalAnd();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        psuedo.push_back(makeInstruction(Opcode::OR, op.line));
    }

    return psuedo;
}

Bytecode Parser::logicalAnd()
{
    //print("and function");
    Bytecode psuedo = equality();

    while (match(TokenType::AND)) {
        Token op = previous();
        Bytecode right = equality();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        psuedo.push_back(makeInstruction(Opcode::AND, op.line));
    }

    return psuedo;
}

Bytecode Parser::equality()
{
    //print("equality function");
    Bytecode psuedo = comparison();

    while (match(TokenType::EQUAL_EQUAL) || match(TokenType::BANG_EQUAL)) {
        Token op = previous();
        Bytecode right = comparison();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        psuedo.push_back(
            makeInstruction(op.type == TokenType::EQUAL_EQUAL ? Opcode::EQUAL : Opcode::NOT_EQUAL,
                            op.line));
    }

    return psuedo;
}

Bytecode Parser::comparison()
{
    //print("comparison function");
    Bytecode psuedo = addition();

    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) || match(TokenType::LESS)
           || match(TokenType::LESS_EQUAL)) {
        Token op = previous();
        Bytecode right = addition();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        switch (op.type) {
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

    return psuedo;
}

Bytecode Parser::addition()
{
    //print("add function");
    Bytecode psuedo = multiplication();

    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = previous();
        Bytecode right = multiplication();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        psuedo.push_back(
            makeInstruction(op.type == TokenType::PLUS ? Opcode::ADD : Opcode::SUBTRACT, op.line));
    }

    return psuedo;
}

Bytecode Parser::multiplication()
{
    //print("mul function");
    Bytecode psuedo = unary();

    while (match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::MODULUS)) {
        Token op = previous();
        Bytecode right = unary();
        psuedo.insert(psuedo.end(), right.begin(), right.end());
        switch (op.type) {
        case TokenType::STAR:
            psuedo.push_back(makeInstruction(Opcode::MULTIPLY, op.line));
            break;
        case TokenType::SLASH:
            psuedo.push_back(makeInstruction(Opcode::DIVIDE, op.line));
            break;
        case TokenType::MODULUS:
            psuedo.push_back(makeInstruction(Opcode::MODULUS, op.line));
            break;
        default:
            break;
        }
    }

    return psuedo;
}

Bytecode Parser::unary()
{
    //print("unary function");
    if (match(TokenType::BANG) || match(TokenType::MINUS)) {
        Token op = previous();
        Bytecode right = unary();
        right.push_back(makeInstruction(Opcode::NOT,
                                        op.line)); // Assuming NOT opcode represents logical negation
        return right;
    }

    return primary();
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

    return psuedo;
}

Bytecode Parser::primary()
{
    //print("primary function");
    Bytecode psuedo;
    Token current = peek();

    switch (current.type) {
    case TokenType::NUMBER:
        psuedo.push_back(Instruction(Opcode::LOAD_VALUE, current.line, std::stoi(current.lexeme)));
        advance();
        break;
    case TokenType::LEFT_PAREN:
        advance();             // Consume '('
        psuedo = expression(); // Parse the expression inside parentheses
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        break;
    case TokenType::VAR:
        varDeclaration();
        advance();
        break;
    case TokenType::IDENTIFIER:
        if (peekNext().type == TokenType::LEFT_PAREN) {
            psuedo = functionCall(peek().lexeme);
        } else {
            varInvoke(); // Handle variable usage
            std::cout <<"Var call";
        }
        break;
    case TokenType::STRING:
        psuedo.push_back(Instruction(Opcode::LOAD_VALUE, current.line, current.lexeme));
        advance();
        break;
    case TokenType::TRUE:
        psuedo.push_back(Instruction(Opcode::LOAD_VALUE, current.line, true));
        advance();
        break;
    case TokenType::FALSE:
        psuedo.push_back(Instruction(Opcode::LOAD_VALUE, current.line, false));
        advance();
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
    //add forloops, attempt, parallel, concurency,
    default:
        // Error handling for unexpected tokens
        // statement();
        error("Unexpected token while in primary.");
    }

    return psuedo;
}

void Parser::varDeclaration()
{
    advance();
    Token identifierToken = peek(); // Expect identifier token after 'var'
    if (identifierToken.type != TokenType::IDENTIFIER) {
        error("Expected identifier after 'var'.");
    }

    // Check if the variable has already been declared
    uint32_t varIndex = symbolTable.getVariableMemoryLocation(identifierToken.lexeme);
   // print(std::to_string(varIndex));
    if ( varIndex > 0) {
        error("Variable already declared.");
    }
    // Declare the variable in the symbol table with a unique memory location
    uint32_t memoryLocation = nextMemoryLocation++;
    symbolTable.declareVariable(identifierToken.lexeme, memoryLocation);
    advance(); // Consume identifier token
    if (match(TokenType::EQUAL)) {
        // Variable assignment
        Bytecode value = expression();
        bytecode.insert(bytecode.end(), value.begin(), value.end());
        // Store the value in the variable's memory location
        bytecode.push_back(makeInstruction(Opcode::STORE_VALUE,
                                        identifierToken.line,
                                        static_cast<int32_t>(memoryLocation)));
    }
    // Optionally handle semicolon for statement termination
    advance();
}

void Parser::varInvoke()
{
    print("var call function");
    Token identifierToken = peek(); // Expect identifier token after variable name

    if (identifierToken.type != TokenType::IDENTIFIER) {
        error("Expected variable name to be an identifier.");
        return;
    }
    print(identifierToken.lexeme);

    // Efficiently look up variable using cached lookup or helper function
    uint32_t memoryLocation = symbolTable.getVariableMemoryLocation(identifierToken.lexeme);

    if (memoryLocation == 0) {
        // Handle variable not found (provide clear error message)
        error("Variable '" + identifierToken.lexeme + "' not found in symbol table.");
        return; // Handle potential errors gracefully
    }

    // Generate appropriate bytecode instruction based on memory location
    bytecode.push_back(makeInstruction(Opcode::LOAD_VALUE, identifierToken.line, static_cast<int32_t>(memoryLocation)));

    // Optionally handle semicolons (if applicable)
    consume(TokenType::SEMICOLON, "Variable call should end in semi colon");
}

Bytecode Parser::functionDeclaration()
{
    Bytecode psuedo;
    consume(TokenType::IDENTIFIER, "Expect function name after 'fn'.");

    Token functionNameToken = previous();
    const std::string functionName = functionNameToken.lexeme;

    // Optionally, handle function overloading and redeclaration checks

    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

    std::vector<std::string> parameters;

    if (!match(TokenType::RIGHT_PAREN)) {
        // Parse function parameters
        do {
            consume(TokenType::IDENTIFIER, "Expect parameter name.");
            parameters.push_back(previous().lexeme);
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    // Parse return type
    std::string returnType = "void"; // Default return type is void
    if (match(TokenType::ARROW)) {
        consume(TokenType::IDENTIFIER, "Expect return type after '->'.");
        returnType = previous().lexeme;
    }

    FunctionInfo functionInfo(nextMemoryLocation++, parameters, returnType);
    declareFunction(functionName, functionInfo);

    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    Bytecode body = block();
    psuedo.push_back(makeInstruction(Opcode::DEFINE_FUNCTION, functionNameToken.line, functionName));
    psuedo.insert(psuedo.end(), body.begin(), body.end());
    psuedo.push_back(makeInstruction(Opcode::RETURN_VALUE, functionNameToken.line));

    return psuedo;
}

Bytecode Parser::functionCall(const std::string &functionName)
{
    Bytecode psuedo;
    FunctionInfo functionInfo = getFunctionInfo(functionName);
    //    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

    //    if (!match(TokenType::RIGHT_PAREN)) {
    //        do {
    //            // Evaluate argument expression
    //            Bytecode arg = expression();
    //            psuedo.insert(psuedo.end(), arg.begin(), arg.end());
    //        } while (match(TokenType::COMMA));
    //    }
    //    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    //    // Generate psuedo for pushing arguments onto the stack
    //    for (const auto &param : functionInfo.parameters) {
    //        psuedo.push_back(makeInstruction(Opcode::PUSH_ARGUMENT, param.line, param.name));
    //    }

    //    // Generate psuedo for calling the function
    //    psuedo.push_back(makeInstruction(Opcode::INVOKE_FUNCTION,
    //                                       static_cast<uint32_t>(peek().line),
    //                                       functionInfo.memoryLocation));

    //    // Optionally, handle passing arguments to the function

    return psuedo;
}
