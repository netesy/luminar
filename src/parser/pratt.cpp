#include "pratt.hh"

#include "../debugger.hh"
#include "../instructions.hh"
#include "../scanner.hh"
#include <chrono>
#include <fstream>
#include <thread>

PrattParser::PrattParser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem)
    : bytecode(Bytecode{})
    , scanner(scanner)
    , variable(typeSystem)
{
    tokens = scanner.scanTokens();
    parse();
}

Bytecode PrattParser::parse()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    scanner.current = 0;
    while (!isAtEnd()) {
        parseExpression();
        if (hadError) {
            // Synchronize here
            synchronize();
            hadError = false;
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Parsing completed in " << duration.count() << " microseconds." << std::endl;
    return bytecode;
}

std::string PrattParser::toString() const
{
    std::string result;
    for (const auto &instruction : bytecode) {
        result += "Instruction: " + instruction.opcodeToString(instruction.opcode)
                  + " | Line: " + std::to_string(instruction.lineNumber) + "\n";
        std::string valueStr;
        std::visit(
            [&valueStr](const auto &val) {
                std::stringstream ss;
                ss << val;
                valueStr = ss.str();
            },
            instruction.value->data);

        result += " | Value: " + valueStr;
        result += "\n";
    }
    return result;
}

ParseFn PrattParser::getParseFn(TokenType type)
{
    // std::cout << "get parsing function: " << peek().lexeme << " with type "
    //           << scanner.tokenTypeToString(type, peek().lexeme) << std::endl;
    switch (type) {
    case TokenType::MINUS:
        return isNewExpression ? &PrattParser::parseUnary : &PrattParser::parseBinary;
    case TokenType::AND:
        return &PrattParser::parseAnd;
    case TokenType::OR:
        return &PrattParser::parseOr;
    case TokenType::BANG:
        return &PrattParser::parseLogical;
    case TokenType::PLUS:
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::MODULUS:
        return &PrattParser::parseBinary;
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
        return &PrattParser::parseComparison;
    case TokenType::PLUS_EQUAL:
    case TokenType::MINUS_EQUAL:
    case TokenType::EQUAL:
        return &PrattParser::parseAssignment;
    case TokenType::NUMBER:
    case TokenType::STRING:
        return &PrattParser::parseLiteral;
    case TokenType::EOF_TOKEN:
        return &PrattParser::parseEOF;
    case TokenType::TRUE:
    case TokenType::FALSE:
        return &PrattParser::parseBoolean;
    case TokenType::VAR:
        return &PrattParser::parseDecVariable;
    case TokenType::FN:
        return &PrattParser::parseFnDeclaration;
    case TokenType::IDENTIFIER:
        return &PrattParser::parseIdentifier;
    case TokenType::LEFT_PAREN:
        return &PrattParser::parseParenthesis;
    case TokenType::LEFT_BRACE:
        return &PrattParser::parseBlock;
    case TokenType::PRINT:
        return &PrattParser::parsePrintStatement;
    case TokenType::IF:
        return &PrattParser::parseIf;
    case TokenType::ELIF:
        return &PrattParser::parseElseIf;
    case TokenType::ELSE:
        return &PrattParser::parseElse;
    case TokenType::WHILE:
        return &PrattParser::parseWhileLoop;
    case TokenType::FOR:
        return &PrattParser::parseForLoop;
    case TokenType::MATCH:
        return &PrattParser::parseMatchStatement;
    case TokenType::CONCURRENT:
        return &PrattParser::parseDeclaration;
    case TokenType::PARALLEL:
        return &PrattParser::parseDeclaration;
    case TokenType::SEMICOLON:
        return &PrattParser::advance;
    case TokenType::INT_TYPE:
    case TokenType::FLOAT_TYPE:
    case TokenType::STR_TYPE:
    case TokenType::BOOL_TYPE:
    case TokenType::USER_TYPE:
    case TokenType::LIST_TYPE:
    case TokenType::DICT_TYPE:
    case TokenType::ARRAY_TYPE:
    case TokenType::ENUM_TYPE:
    case TokenType::FUNCTION_TYPE:
    case TokenType::NIL_TYPE:
    case TokenType::INT8_TYPE:
    case TokenType::INT16_TYPE:
    case TokenType::INT32_TYPE:
    case TokenType::INT64_TYPE:
    case TokenType::UINT_TYPE:
    case TokenType::UINT8_TYPE:
    case TokenType::UINT16_TYPE:
    case TokenType::UINT32_TYPE:
    case TokenType::UINT64_TYPE:
    case TokenType::FLOAT32_TYPE:
    case TokenType::FLOAT64_TYPE:
    case TokenType::SUM_TYPE:
    case TokenType::ANY_TYPE:
    case TokenType::UNION_TYPE:
        return &PrattParser::parseTypes; // Add parsing function for types if needed
    case TokenType::IN:
    case TokenType::THIS:
    case TokenType::ENUM:
    case TokenType::ASYNC:
    case TokenType::AWAIT:
    case TokenType::CLASS:
    case TokenType::SUPER:
    case TokenType::IMPORT:
    case TokenType::RETURN:
    case TokenType::HANDLE:
    case TokenType::DEFAULT:
    case TokenType::ATTEMPT:
        return &PrattParser::advance; // Add parsing function for these keywords if needed
    case TokenType::COMMA:
    case TokenType::DOT:
    case TokenType::COLON:
    case TokenType::QUESTION:
    case TokenType::ARROW:
    case TokenType::LEFT_BRACKET:
    case TokenType::RIGHT_BRACKET:
    case TokenType::RIGHT_BRACE:
        return &PrattParser::advance; // Add parsing function for these delimiters if needed
    case TokenType::UNDEFINED:
        return &PrattParser::parseUnexpected; // Handle undefined tokens if needed
    default:
        return nullptr;
    }
}

void PrattParser::advance()
{
    if (current < tokens.size() - 1) { //added -1
        current++;
    }
}

Token PrattParser::peek()
{
    return tokens[current];
}

Token PrattParser::peekNext()
{
    if (current + 1 < tokens.size()) {
        return tokens[current + 1];
    } else {
        return Token{TokenType::EOF_TOKEN,
                     "",
                     scanner.getFilename(),
                     scanner.getFilepath(),
                     static_cast<int>(current),
                     scanner.getLine()};
    }
}

bool PrattParser::isAtEnd()
{
    return tokens[current].type == TokenType::EOF_TOKEN;
}

Token PrattParser::previous()
{
    //    return tokens[current - 1];
    if (current > 0) {
        return tokens[current - 1];
    }
    return tokens[0]; // Return the first token if there is no previous one
}

bool PrattParser::check(TokenType type)
{
    if (scanner.isAtEnd())
        return false;
    if (peek().type == type) {
        return true;
    }
    return false;
}

bool PrattParser::match(TokenType type)
{
    if (scanner.isAtEnd())
        return false;
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

void PrattParser::consume(TokenType type, const std::string &message)
{
    if (!match(type)) {
        error(message);
    }
}

bool PrattParser::isExpression(TokenType type)
{
    switch (type) {
    case TokenType::NUMBER:
    case TokenType::STRING:
    case TokenType::TRUE:
    case TokenType::FALSE:
        return true;
    case TokenType::IDENTIFIER:
        return true;
    case TokenType::LEFT_PAREN:
        return true;
    case TokenType::MINUS:
    case TokenType::BANG:
    case TokenType::PLUS:
    case TokenType::MODULUS:
    case TokenType::SLASH:
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
        return true;
    default:
        return false;
    }
}

void PrattParser::error(const std::string &message)
{
    hadError = true;
    Debugger::error(message, peek(), InterpretationStage::PARSING, scanner.getSource());
}

void PrattParser::PrattParser::synchronize()
{
    advance();

    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON)
            return;

        switch (peek().type) {
        case TokenType::CLASS:
        case TokenType::FN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::ELIF:
        case TokenType::ELSE:
        case TokenType::ATTEMPT:
        case TokenType::CONCURRENT:
        case TokenType::PARALLEL:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;
        }

        advance();
    }
}

void PrattParser::parsePrecedence(Precedence precedence)
{
    ParseFn prefixParseFn = getParseFn(peek().type);
    if (prefixParseFn == nullptr) {
        error("Unexpected token");
        return;
    }

    bool isStatement = (prefixParseFn == &PrattParser::parsePrintStatement
                        || prefixParseFn == &PrattParser::parseIfStatement
                        || prefixParseFn == &PrattParser::parseWhileLoop
                        || prefixParseFn == &PrattParser::parseForLoop);

    if (!isStatement) {
        advance(); // Only advance for non-statement expressions
    }

    (this->*prefixParseFn)();
    isNewExpression = false;

    // Only continue parsing for expressions, not statements
    if (!isStatement) {
        while (precedence <= getTokenPrecedence(peek().type)) {
            if (isAtEnd()) {
                break;
            }

            ParseFn infixParseFn = getParseFn(peek().type);
            if (infixParseFn == nullptr) {
                break;
            }

            advance();
            (this->*infixParseFn)();
        }
    }
}

void PrattParser::parseEOF()
{
    Token op = peek();
    if (match(TokenType::EOF_TOKEN)) {
        std::cout << "Unexpected end of token" << std::endl;
        emit(Opcode::HALT, op.line);
        return;
    }
}

void PrattParser::parseUnexpected()
{
    Token token = peek();
    error("Unexpected token when getting parseFN: " + token.lexeme);
    advance(); // Consume the unexpected token
}

Precedence PrattParser::getTokenPrecedence(TokenType type)
{
    switch (type) {
    case TokenType::OR:
        return PREC_OR;
    case TokenType::AND:
        return PREC_AND;
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
        return PREC_EQUALITY;
    case TokenType::PLUS:
    case TokenType::MINUS:
        return PREC_TERM;
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::MODULUS:
        return PREC_FACTOR;
    case TokenType::BANG:
        return PREC_UNARY;
    case TokenType::LEFT_PAREN:
    case TokenType::DOT:
        return PREC_CALL;
    case TokenType::NUMBER:
    case TokenType::STRING:
    case TokenType::IDENTIFIER:
    case TokenType::TRUE:
    case TokenType::FALSE:
        return PREC_PRIMARY;
    default:
        return PREC_NONE;
    }
}

Instruction PrattParser::emit(Opcode opcode, uint32_t lineNumber)
{
    Instruction instruction(opcode, lineNumber);
    instruction.debug();
    bytecode.push_back(instruction);
    return instruction;
}

Instruction PrattParser::emit(Opcode opcode, uint32_t lineNumber, Value &&value)
{
    ValuePtr valuePtr = std::make_shared<Value>(std::move(value));
    Instruction instruction(opcode, lineNumber, valuePtr);
    instruction.debug();
    bytecode.push_back(instruction);
    return instruction;
}

void PrattParser::parsePrimary()
{
    TokenType tokenType = peek().type;
    if (tokenType == TokenType::NUMBER || tokenType == TokenType::STRING) {
        parseLiteral();
    } else if (tokenType == TokenType::TRUE || tokenType == TokenType::FALSE) {
        parseBoolean();
    } else if (tokenType == TokenType::IDENTIFIER) {
        parseIdentifier();
    } else if (tokenType == TokenType::LEFT_PAREN) {
        parseParenthesis();
    } else if (tokenType == TokenType::MINUS || tokenType == TokenType::PLUS) {
        parseUnary();
    } else if (isAtEnd()) { //isAtEnd()
        parseEOF();
    } else {
        error("Unexpected token in primary expression");
    }
}

void PrattParser::parseExpression()
{
    //    ParseFn parseFn = getParseFn(peek().type);
    //    if (parseFn == &PrattParser::parsePrintStatement || parseFn == &PrattParser::parseIfStatement
    //        || parseFn == &PrattParser::parseWhileLoop || parseFn == &PrattParser::parseForLoop) {
    //        (this->*parseFn)();
    //    } else {
    //        parsePrecedence(PREC_ASSIGNMENT);
    //    }
    if (check(TokenType::IF) || check(TokenType::ELIF) || check(TokenType::ELSE)) {
        parseIfStatement();
    } else if (match(TokenType::WHILE)) {
        parseWhileLoop();
    } else if (match(TokenType::FOR)) {
        parseForLoop();
    } else {
        parsePrecedence(PREC_ASSIGNMENT);
    }
}

void PrattParser::parseDeclaration()
{
    if (check(TokenType::VAR)) {
        parseDecVariable();
    } else if (match(TokenType::FN)) {
        parseFnDeclaration();
    } else {
        parseStatement();
    }
}

void PrattParser::parseStatement()
{
    if (check(TokenType::PRINT)) {
        parsePrintStatement();
    } else if (check(TokenType::LEFT_BRACE)) {
        parseBlock();
    } else if (check(TokenType::IF)) {
        parseIfStatement();
    } else if (check(TokenType::ELIF)) {
        parseIfStatement();
    } else if (check(TokenType::WHILE)) {
        parseWhileLoop();
    } else if (check(TokenType::FOR)) {
        parseForLoop();
    } else if (check(TokenType::MATCH)) {
        parseMatchStatement();
    } else if (check(TokenType::IDENTIFIER) && peekNext().type == TokenType::EQUAL) {
        parseAssignment();
    } else {
        parseExpressionStatement();
    }
}

void PrattParser::parseExpressionStatement()
{
    parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
}

void PrattParser::parseParenthesis()
{
    //advance(); // Consume '('
    parseExpression(); // Parse the expression inside parentheses
    consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
}

void PrattParser::parseUnary()
{
    Token op = previous();
    advance(); // Consume the unary operator
    parsePrecedence(PREC_UNARY);
    if (op.type == TokenType::MINUS) {
        emit(Opcode::NEGATE, op.line);
    } else if (op.type == TokenType::BANG) {
        emit(Opcode::NOT, op.line);
    }
}

void PrattParser::parseBoolean()
{
    Token token = previous();
    if (token.type == TokenType::TRUE) {
        emit(Opcode::LOAD_CONST, token.line, Value{std::make_shared<Type>(TypeTag::Bool), true});
    } else if (token.type == TokenType::FALSE) {
        emit(Opcode::LOAD_CONST, token.line, Value{std::make_shared<Type>(TypeTag::Bool), false});
    } else {
        error("Unexpected boolean value");
    }
}

void PrattParser::parseBinary()
{
    Token op = previous();
    Precedence precedence = getTokenPrecedence(op.type);
    parsePrecedence(static_cast<Precedence>(precedence + 1));
    switch (op.type) {
    case TokenType::PLUS:
        emit(Opcode::ADD, op.line);
        break;
    case TokenType::MINUS:
        emit(Opcode::SUBTRACT, op.line);
        break;
    case TokenType::STAR:
        emit(Opcode::MULTIPLY, op.line);
        break;
    case TokenType::SLASH:
        emit(Opcode::DIVIDE, op.line);
        break;
    case TokenType::MODULUS:
        emit(Opcode::MODULUS, op.line);
        break;
    default:
        error("Unexpected binary operator");
    }
}

void PrattParser::parseLiteral()
{
    Token token = previous();
    TypePtr typePtr = std::make_shared<Type>(inferType(token));
    switch (token.type) {
    case TokenType::NUMBER: {
        Value value = setValue(typePtr, token.lexeme);
        emit(Opcode::LOAD_CONST, token.line, std::move(value));
    } break;
    case TokenType::STRING:
        parseString();
        break;
    default:
        error("Unexpected literal type");
    }
}

void PrattParser::parseString()
{
    std::string str = previous().lexeme;
    TypePtr typePtr = std::make_shared<Type>(inferType(previous()));
    // Check if the string contains any {} pairs
    bool isInterpolated = (str.find('{') != std::string::npos)
                          && (str.find('}') != std::string::npos);
    if (!isInterpolated) {
        // Regular string, just emit a LOAD_CONST
        emit(Opcode::LOAD_STR, previous().line, setValue(typePtr, str));
        return;
    }
    std::string current;
    bool inExpression = false;
    int bracketCount = 0;
    int partCount = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        if (c == '{' && !inExpression) {
            if (!current.empty()) {
                emit(Opcode::LOAD_STR, previous().line, setValue(typePtr, current));
                partCount++;
            }
            current.clear();
            inExpression = true;
            bracketCount = 1;
        } else if (c == '{' && inExpression) {
            bracketCount++;
            current += c;
        } else if (c == '}' && inExpression) {
            bracketCount--;
            if (bracketCount == 0) {
                // Parse the expression inside the brackets
                parseExpression();
                partCount++;
                current.clear();
                inExpression = false;
            } else {
                current += c;
            }
        } else if (c == '\\' && i + 1 < str.length()) {
            // Handle escape sequences
            char nextChar = str[i + 1];
            if (nextChar == '{' || nextChar == '}') {
                current += nextChar;
                i++; // Skip the next character
            } else {
                current += c;
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        emit(Opcode::LOAD_STR, previous().line, setValue(typePtr, current));
        partCount++;
    }
    // Emit the INTERPOLATE_STRING instruction with the part count
    emit(Opcode::INTERPOLATE_STRING,
         previous().line,
         Value{std::make_shared<Type>(TypeTag::Int), partCount});
}

void PrattParser::parseIdentifier()
{
    Token nameToken = peek();     // The identifier token
    Token nextToken = peekNext(); // Look ahead at the next token

    if (nextToken.type == TokenType::EQUAL) {
        // Handle assignment
        parseAssignment();
    } else if (nextToken.type == TokenType::LEFT_PAREN) {
        // Handle function call
        parseFnCall();
    } else {
        // Handle variable load
        parseLoadVariable();
    }
}

void PrattParser::parseDecVariable()
{
    // Parse variable declaration with type
    // consume(TokenType::VAR, "Expected 'var' before variable name");
    Token name = peek();
    TypeTag type = TypeTag::Any;
    consume(TokenType::IDENTIFIER, "Expected variable name after 'var' token");
    if (check(TokenType::COLON)) {
        consume(TokenType::COLON, "Expected ':' after variable name");
        Token typeToken = peek();
        type = stringToType(typeToken.lexeme);
        std::cout << "Type: " << typeToken.lexeme << std::endl;
        advance();
        //consume(TokenType::IDENTIFIER, "Expected type after ':'"); // edit this to get every type of type
    }
    consume(TokenType::EQUAL, "Expected '=' after type");
    parseExpression();
    // Check if the next token is a semicolon
    if (match(TokenType::SEMICOLON)) {
        // If it is, consume the semicolon without emitting any opcodes
        advance();
    }

    // Value value{std::make_shared<Type>(inferType())
    declareVariable(name, std::make_shared<Type>(type));
    int32_t memoryLocation = getVariableMemoryLocation(name);
    emit(Opcode::STORE_VARIABLE,
         name.line,
         Value{std::make_shared<Type>(TypeTag::Int), memoryLocation});
}

void PrattParser::parseLoadVariable()
{
    // Parse loading existing variable
    Token name = previous();
    if (check(TokenType::LEFT_PAREN)) {
        parseFnCall();
    }

    // consume(TokenType::SEMICOLON, "Expected ';' after var load.");
    std::cout << name.lexeme << std::endl;
    int32_t memoryLocation = getVariableMemoryLocation(name);
    emit(Opcode::LOAD_VARIABLE,
         name.line,
         Value{std::make_shared<Type>(TypeTag::Int), memoryLocation});
}

void PrattParser::parseBlock()
{
    enterScope();
    // consume(TokenType::LEFT_BRACE, "Expected '{' at the start of a block");

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (check(TokenType::LEFT_BRACE)) {
            // Nested block
            parseBlock();
        } else {
            parseStatement();
        }
    }

    if (!isAtEnd()) {
        consume(TokenType::RIGHT_BRACE, "Expected '}' at the end of a block");
    } else {
        error("Unexpected end of file inside a block");
    }
}

void PrattParser::parseAssignment()
{
    Token token = tokens[current];
    //    advance();
    std::string varName = token.lexeme;

    // Parse the expression to be assigned to the variable
    parsePrecedence(PREC_ASSIGNMENT);

    // Check if the variable exists in any scope
    if (!variable.hasVariable(varName)) {
        // Variable is not declared, emit an error
        error("Undeclared variable: " + varName);
        return;
    }

    // Get the memory location of the variable (assumes variable is now declared)
    int32_t memoryLocation = variable.getVariableMemoryLocation(varName);
    emit(Opcode::STORE_VARIABLE,
         token.line,
         Value{std::make_shared<Type>(TypeTag::Int), memoryLocation});
}

void PrattParser::parseAnd()
{
    Token op = previous();
    parsePrecedence(static_cast<Precedence>(PREC_AND + 1));
    emit(Opcode::AND, op.line);
}

void PrattParser::parseOr()
{
    Token op = previous();
    parsePrecedence(static_cast<Precedence>(PREC_OR + 1));
    emit(Opcode::OR, op.line);
}

void PrattParser::parseLogical()
{
    Token op = previous();
    parsePrecedence(PREC_OR);
    if (op.type == TokenType::BANG) {
        emit(Opcode::NOT, op.line);
    }
}

void PrattParser::parseComparison()
{
    Token op = previous(); //change and or and comparison to previous
    Precedence precedence = getTokenPrecedence(op.type);

    parsePrecedence(static_cast<Precedence>(precedence + 1));

    switch (op.type) {
    case TokenType::EQUAL_EQUAL:
        emit(Opcode::EQUAL, op.line);
        break;
    case TokenType::BANG_EQUAL:
        emit(Opcode::NOT_EQUAL, op.line);
        break;
    case TokenType::LESS:
        emit(Opcode::LESS_THAN, op.line);
        break;
    case TokenType::LESS_EQUAL:
        emit(Opcode::LESS_THAN_OR_EQUAL, op.line);
        break;
    case TokenType::GREATER:
        emit(Opcode::GREATER_THAN, op.line);
        break;
    case TokenType::GREATER_EQUAL:
        emit(Opcode::GREATER_THAN_OR_EQUAL, op.line);
        break;
    default:
        error("Unexpected comparison operator");
    }
}

void PrattParser::parsePrintStatement()
{
    Token op = peek();
    //consume(TokenType::PRINT, "Expected 'print' before print expression.");
    parseExpression();
    emit(Opcode::PRINT, op.line);
    std::cout << "Previous Token print: " << previous().lexeme << std::endl;
    std::cout << "Current Token print: " << peek().lexeme << std::endl;
    if (!check(TokenType::RIGHT_BRACE) && previous().type != TokenType::SEMICOLON) {
        consume(TokenType::SEMICOLON, "Expected ';' after print function.");
    }
}

void PrattParser::parseIfStatement()
{
    std::cout << "Current Token if: " << previous().lexeme << std::endl;
    // Handle 'if' condition and 'then' block
    if (previous().type == TokenType::IF) {
        parseIf();
    }

    std::cout << "Current Token elif: " << peek().lexeme << std::endl;
    // Handle 'elif' blocks if they exist
    while (check(TokenType::ELIF)) {
        parseElseIf();
    }

    std::cout << "Current Token else: " << peek().lexeme << std::endl;
    // Handle 'else' block if it exists
    if (check(TokenType::ELSE)) {
        parseElse();
    }

    std::cout << "Finished parsing if statement" << std::endl;
}

void PrattParser::parseIf()
{
    //consume(TokenType::IF, "Expected IF Token");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition");

    // Emit JUMP_IF_FALSE with a placeholder for patching later
    size_t thenJump = bytecode.size();
    std::cout << "Emitting JUMP_IF_FALSE at " << thenJump << std::endl;
    emit(Opcode::JUMP_IF_FALSE,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int32), 0}); // Placeholder

    // Parse the 'then' block
    std::cout << "Parsing 'then' block" << std::endl;
    parseBlock();

    // Emit JUMP with a placeholder to jump to end of else block
    size_t elseJump = bytecode.size();
    std::cout << "Emitting JUMP at " << elseJump << std::endl;
    emit(Opcode::JUMP,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int32), 0}); // Jump to end (placeholder)

    // Patch thenJump to jump to the else block if condition is false
    int32_t thenOffset = bytecode.size() - thenJump - 1;
    std::cout << "Patching JUMP_IF_FALSE at " << thenJump << " with offset " << thenOffset
              << std::endl;
    bytecode[thenJump].value = std::make_shared<Value>(
        Value{std::make_shared<Type>(TypeTag::Int32), thenOffset});

    // Store elseJump for patching later
    endJumps.push_back(elseJump);
}

void PrattParser::parseElseIf()
{
    //consume(TokenType::ELIF, "Expected ELIF Token");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'elif'");
    parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after elif condition");

    // Emit JUMP_IF_FALSE with a placeholder for patching later
    size_t thenJump = bytecode.size();
    std::cout << "Emitting JUMP_IF_FALSE at " << thenJump << std::endl;
    emit(Opcode::JUMP_IF_FALSE,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int32), 0}); // Placeholder

    // Parse the 'elif' block
    std::cout << "Parsing 'elif' block" << std::endl;
    parseBlock();

    // Emit JUMP with a placeholder to jump to end of else block
    size_t elseJump = bytecode.size();
    std::cout << "Emitting JUMP at " << elseJump << std::endl;
    emit(Opcode::JUMP,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int32), 0}); // Jump to end (placeholder)

    // Patch thenJump to jump to the next block if condition is false
    int32_t thenOffset = bytecode.size() - thenJump - 1;
    std::cout << "Patching JUMP_IF_FALSE at " << thenJump << " with offset " << thenOffset
              << std::endl;
    bytecode[thenJump].value = std::make_shared<Value>(
        Value{std::make_shared<Type>(TypeTag::Int32), thenOffset});

    // Store elseJump for patching later
    endJumps.push_back(elseJump);
}

void PrattParser::parseElse()
{
    // consume(TokenType::ELSE, "Expected ELSE Token");
    std::cout << "Parsing 'else' block" << std::endl;
    parseBlock();

    // Patch all endJumps to jump to the end of the 'else' block
    for (size_t jump : endJumps) {
        int32_t endOffset = bytecode.size() - jump - 1;
        std::cout << "Patching JUMP at " << jump << " with offset " << endOffset << std::endl;
        bytecode[jump].value = std::make_shared<Value>(
            Value{std::make_shared<Type>(TypeTag::Int32), endOffset});
    }

    // Clear endJumps after patching
    endJumps.clear();
}

void PrattParser::parseWhileLoop()
{
    size_t loopStart = bytecode.size(); // Start of the loop condition
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");

    size_t conditionJump = bytecode.size();
    emit(Opcode::JUMP_IF_FALSE,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int32),
               0}); // Placeholder for the jump out of the loop

    parseBlock();

    int32_t jmpLoc = loopStart - bytecode.size() - 1;
    emit(Opcode::JUMP,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int32),
               jmpLoc}); // Jump back to the start of the loop condition

    int32_t conditionJumpOffset = bytecode.size() - conditionJump - 1;
    bytecode[conditionJump].value = std::make_shared<Value>(
        Value{std::make_shared<Type>(TypeTag::Int),
              conditionJumpOffset}); // Update the jump condition to exit the loop
}

void PrattParser::parseForLoop()
{
    // Token op = previous();
    // consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'");

    // parseExpression();
    // consume(TokenType::SEMICOLON, "Expected ';' after loop initializer");
    // parseExpression();
    // consume(TokenType::SEMICOLON, "Expected ';' after loop condition");

    // size_t conditionOffset = bytecode.size();

    // parseExpression();
    // consume(TokenType::RIGHT_PAREN, "Expected ')' after loop increment");

    // size_t bodyJump = bytecode.size();emit(Opcode::JUMP_IF_FALSE, op.line, 0);
    // parseExpression();
    // emit(Opcode::JUMP, op.line, (int32_t)(conditionOffset - bytecode.size() - 1));
}

void PrattParser::parseMatchStatement()
{
    Token op = previous();
    parseExpression();
    emit(Opcode::PATTERN_MATCH, op.line);

    parseExpression();
    emit(Opcode::PATTERN_MATCH, op.line);
}

void PrattParser::parseConcurrentStatement()
{
    consume(TokenType::CONCURRENT, "Expected 'concurrent' keyword");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'concurrent'");

    // Parse arguments (if any)
    while (!check(TokenType::RIGHT_PAREN)) {
        parseExpression(); // Parse each argument
        if (match(TokenType::COMMA)) {
            continue; // Allow multiple arguments separated by commas
        }
    }

    consume(TokenType::RIGHT_PAREN, "Expected ')' after concurrent arguments");

    parseBlock(); // Parse the block of statements to be executed concurrently
}

void PrattParser::parseParallelStatement()
{
    consume(TokenType::PARALLEL, "Expected 'parallel' keyword");
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'parallel'");

    // Parse arguments (if any)
    while (!check(TokenType::RIGHT_PAREN)) {
        parseExpression(); // Parse each argument
        if (match(TokenType::COMMA)) {
            continue; // Allow multiple arguments separated by commas
        }
    }

    consume(TokenType::RIGHT_PAREN, "Expected ')' after parallel arguments");

    parseBlock(); // Parse the block of statements to be executed in parallel
}

void PrattParser::parseFnDeclaration()
{
    //    //consume(TokenType::FN, "Expected 'fn' keyword to start function definition");
    //    Token name = previous();
    //    // consume(TokenType::IDENTIFIER, "Expected function name");

    //    // Parse optional parameters
    //    std::vector<std::pair<std::string, std::optional<std::string>>> parameters;
    //    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    //    if (peek().type != TokenType::RIGHT_PAREN) {
    //        do {
    //            Token paramName = peek();
    //            consume(TokenType::IDENTIFIER, "Expected parameter name");
    //            std::optional<std::string> defaultValue;
    //            if (match(TokenType::COLON)) {
    //                Token paramType = peek();
    //                TypeTag types = stringToType(paramType.lexeme);
    //                consume(TokenType::IDENTIFIER, "Expected type after ':' in parameter declaration");
    //                if (match(TokenType::EQUAL)) {
    //                    defaultValue = peek().lexeme;
    //                    advance();
    //                } else {
    //                    error("Expected '=' after type in parameter declaration with default "
    //                          "value");
    //                }
    //            }
    //            //            Token paramName = consume(TokenType::IDENTIFIER, "Expect parameter name.");
    //            //            Token paramType = consume(TokenType::IDENTIFIER, "Expect parameter type.");
    //            //            ReturnType type = stringToReturnType(paramType.lexeme);
    //            parameters.push_back(std::make_pair(paramName.lexeme, defaultValue));
    //            declareVariable(paramName,
    //                            types,
    //                            defaultValue); // Declare parameter as a variable in the function scope
    //        } while (match(TokenType::COMMA));
    //    }
    //    consume(TokenType::RIGHT_PAREN, "Expected ')' after function parameters");

    //    // Parse expected return type (optional)
    //    std::optional<std::string> returnType;
    //    if (match(TokenType::COLON)) {
    //        consume(TokenType::IDENTIFIER, "Expected return type after ':'");
    //        returnType = peek().lexeme;
    //        advance();
    //    }

    //    // Function body parsing logic goes here
    //    parseBlock();
    //    //    consume(TokenType::LEFT_BRACE, "Expected '{' before function body");
    //    //    parseDeclaration();
    //    //    consume(TokenType::RIGHT_BRACE, "Expected '}' after function body");

    //    // Emit bytecode for function declaration
    //    emit(Opcode::DEFINE_FUNCTION,
    //         name.line,
    //         Value{std::make_shared<Type>(TypeTag::String), name.lexeme});
}

void PrattParser::parseFnCall()
{
    //    Token name = previous();
    //    //consume(TokenType::IDENTIFIER, "Expected function name for call");
    //    consume(TokenType::LEFT_PAREN, "Expected '(' after function name in call");

    //    // Parse arguments (optional)
    //    Value arguments;
    //    if (peek().type != TokenType::RIGHT_PAREN) {
    //        do {
    //            parseExpression();
    //            arguments.push_back(previous().lexeme);
    //        } while (match(TokenType::COMMA));
    //    }
    //    consume(TokenType::RIGHT_PAREN, "Expected ')' after function call arguments");
    //    // Emit bytecode for function call with arguments
    //    for (const auto &arg : arguments) {
    //        emit(Opcode::PUSH_ARGS, name.line, Value{std::make_shared<Type>(TypeTag::String), arg});
    //    }

    //    // Emit bytecode for function call with arguments
    //    emit(Opcode::INVOKE_FUNCTION,
    //         name.line,
    //         Value{std::make_shared<Type>(TypeTag::String), name.lexeme});
}

void PrattParser::parseImport()
{
    //    consume(TokenType::IMPORT, "Expected 'import' keyword");

    //    std::vector<std::string> importPath;

    //    do {
    //        if (check(TokenType::IDENTIFIER)) {
    //            importPath.push_back(current().lexeme);
    //            advance();
    //        } else if (match(TokenType::STRING)) {
    //            importPath.push_back(previous().literal.toString());
    //        } else {
    //            error(current(), "Expected module name or string literal in import statement");
    //            return;
    //        }

    //        // Handle submodules
    //        if (match(TokenType::DOT)) {
    //            if (!check(TokenType::IDENTIFIER)) {
    //                error(current(), "Expected identifier after '.' in import statement");
    //                return;
    //            }
    //        }
    //    } while (match(TokenType::DOT));

    //    // Now importPath contains the complete module path

    //    // Perform semantic checks
    //    std::string moduleName = ""; // Construct the full module name
    //    for (const auto &component : importPath) {
    //        if (!moduleName.empty()) {
    //            moduleName += ".";
    //        }
    //        moduleName += component;

    //        // Check if moduleName exists and is accessible
    //        //        if (!moduleExists(moduleName)) {
    //        //            error(current(), "Module '" + moduleName + "' not found or inaccessible");
    //        //            return;
    //        //        }
    //    }

    //    // Example logic: Print the imported module path
    //    std::cout << "Imported module path: ";
    //    for (const auto &pathComponent : importPath) {
    //        std::cout << pathComponent << ".";
    //    }
    //    std::cout << std::endl;
}

void PrattParser::parseModules()
{
    //    consume(TokenType::MODULE, "Expected 'module' keyword");
    //    if (check(TokenType::IDENTIFIER)) {
    //        std::string moduleName = peek().lexeme;
    //        consume(TokenType::IDENTIFIER, "Expected module name")
    //    }
    //    consume(TokenType::LEFT_BRACE, "Expected '{' after module name");

    //    // Enter the module scope
    //    enterScope(moduleName);

    //    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
    //        parseDeclaration(); // Parse declarations within the module
    //    }

    //    consume(TokenType::RIGHT_BRACE, "Expected '}' after module contents");
    //    consume(TokenType::SEMICOLON, "Expected ';' after module declaration");

    //    // Exit the module scope
    //    exitScope();
}

void PrattParser::parseTypes()
{
    inferType(peek());
}

std::vector<Instruction> PrattParser::getBytecode() const
{
    // std::cout << "Getting the bytecode: " << std::endl;
    // Get the generated bytecode
    return bytecode;
}
