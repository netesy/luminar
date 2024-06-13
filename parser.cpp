#include "parser.hh"
#include "debugger.hh"
#include "instructions.hh"
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
    for (const auto &instruction : bytecode) {
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
    return result;
}

ParseFn Parser::getParseFn(TokenType type)
{
    switch (type) {
    case TokenType::MINUS:
        return isNewExpression ? &Parser::parseUnary : &Parser::parseBinary;
    case TokenType::AND:
        return &Parser::parseAnd;
    case TokenType::OR:
        return &Parser::parseOr;
    case TokenType::BANG:
        return &Parser::parseLogical;
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
    case TokenType::PLUS_EQUAL:
    case TokenType::MINUS_EQUAL:
    case TokenType::EQUAL:
        return &Parser::parseAssignment;
    case TokenType::NUMBER:
    case TokenType::STRING:
        return &Parser::parseLiteral;
    case TokenType::EOF_TOKEN:
        return &Parser::parseEOF;
    case TokenType::TRUE:
    case TokenType::FALSE:
        return &Parser::parseBoolean;
    case TokenType::VAR:
        return &Parser::parseDecVariable;
    case TokenType::FN:
        return &Parser::parseFnDeclaration;
    case TokenType::IDENTIFIER:
        return &Parser::parseLoadVariable;
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
    if (current + 1 < tokens.size()) {
        return tokens[current + 1];
    } else {
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
    if (peek().type == type) {
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
    if (!match(type)) {
        error(message);
    }
}

bool Parser::isExpression(TokenType type)
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

void Parser::error(const std::string &message)
{
    Debugger::error(message,
                    peek().line,
                    current,
                    InterpretationStage::PARSING,
                    scanner.tokenTypeToString(peek().type, peek().lexeme));
}

void Parser::parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixParseFn = getParseFn(previous().type);

    if (prefixParseFn == nullptr) {
        error("Unexpected token");
        return;
    }

    (this->*prefixParseFn)();
    isNewExpression = false;

    while (precedence <= getTokenPrecedence(peek().type)) {
        if (isAtEnd()) {
            break;
        }

        advance();
        ParseFn infixParseFn = getParseFn(previous().type);

        if (infixParseFn == nullptr) {
            error("Unexpected token");
            return;
        }

        (this->*infixParseFn)();
    }
}

void Parser::parseEOF()
{
    Token op = peek();
    if (match(TokenType::EOF_TOKEN)) {
        std::cout << "Unexpected end of token" << std::endl;
        emit(Opcode::HALT, op.line);
        return;
    }
}

Precedence Parser::getTokenPrecedence(TokenType type)
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

Instruction Parser::emit(Opcode opcode, uint32_t lineNumber)
{
    Instruction instruction(opcode, lineNumber);
    bytecode.push_back(instruction);
    return instruction;
}

Instruction Parser::emit(Opcode opcode,
                         uint32_t lineNumber,
                         std::variant<int32_t, double, bool, std::string> value)
{
    Instruction instruction(opcode, lineNumber, value);
    bytecode.push_back(instruction);
    return instruction;
}

void Parser::parsePrimary()
{
    TokenType tokenType = peek().type;
    if (tokenType == TokenType::NUMBER || tokenType == TokenType::STRING) {
        parseLiteral();
    } else if (tokenType == TokenType::TRUE || tokenType == TokenType::FALSE) {
        parseBoolean();
    } else if (tokenType == TokenType::IDENTIFIER) {
        parseDecVariable();
    } else if (tokenType == TokenType::LEFT_PAREN) {
        parseParenthesis();
    } else {
        error("Unexpected token in primary expression");
    }
}

void Parser::parseExpression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

void Parser::parseDeclaration()
{
    if (match(TokenType::VAR)) {
        parseDecVariable();
    } else {
        parseStatement();
    }
}

void Parser::parseStatement()
{
    if (match(TokenType::PRINT)) {
        parsePrintStatement();
    } else if (match(TokenType::LEFT_BRACE)) {
        parseBlock();
    } else if (match(TokenType::IF)) {
        parseIfStatement();
    } else if (match(TokenType::WHILE)) {
        parseWhileLoop();
    } else if (match(TokenType::FOR)) {
        parseForLoop();
    } else if (match(TokenType::MATCH)) {
        parseMatchStatement();
    } else {
        parseExpressionStatement();
    }
}

void Parser::parseExpressionStatement()
{
    parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
}

void Parser::parseParenthesis()
{
    // advance(); // Consume '('

    parseExpression(); // Parse the expression inside parentheses

    consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
}

void Parser::parseUnary()
{
    Token op = previous();
    parsePrecedence(PREC_UNARY);
    if (op.type == TokenType::MINUS) {
        emit(Opcode::NEGATE, op.line);
    } else if (op.type == TokenType::BANG) {
        emit(Opcode::NOT, op.line);
    }
}

void Parser::parseBinary()
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

void Parser::parseLiteral()
{
    Token token = previous();
    switch (token.type) {
    case TokenType::NUMBER:
        emit(Opcode::LOAD_CONST, token.line, std::stod(token.lexeme));
        break;
    case TokenType::STRING:
        emit(Opcode::LOAD_STR, token.line, token.lexeme);
        break;
    default:
        error("Unexpected literal type");
    }
}

void Parser::parseBoolean()
{
    Token token = previous();
    if (token.type == TokenType::TRUE) {
        emit(Opcode::LOAD_CONST, token.line, true);
    } else if (token.type == TokenType::FALSE) {
        emit(Opcode::LOAD_CONST, token.line, false);
    } else {
        error("Unexpected boolean value");
    }
}

void Parser::parseDecVariable()
{
    // Parse variable declaration with type
    Token name = peek();
    consume(TokenType::IDENTIFIER, "Expected 'var' before variable name");
    if (check(TokenType::COLON)) {
        consume(TokenType::COLON, "Expected ':' after variable name");
        Token typeToken = peek();
        advance();
        //consume(TokenType::IDENTIFIER, "Expected type after ':'"); // edit this to get every type of type
    }
    consume(TokenType::EQUAL, "Expected '=' after type");
    parseExpression();
    declareVariable(name);
    int32_t memoryLocation = getVariableMemoryLocation(name);
    emit(Opcode::STORE_VARIABLE, name.line, memoryLocation);
}

void Parser::parseLoadVariable()
{
    // Parse loading existing variable
    Token name = previous();

    int32_t memoryLocation = getVariableMemoryLocation(name);

    if (memoryLocation) {
        emit(Opcode::LOAD_VARIABLE, name.line, memoryLocation);
    } else if(check(TokenType::LEFT_PAREN)) {
        parseFnCall();
    }else{
        error("Undeclared variable: " + name.lexeme);
    }
}

void Parser::parseBlock()
{
    enterScope();
    consume(TokenType::LEFT_BRACE, "Expected '{' before block.");
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        parseDeclaration(); // Handles statements inside the block
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after block.");
    exitScope();
}

void Parser::parseAssignment()
{
    Token token = previous();
    std::string varName = token.lexeme;

    parsePrecedence(PREC_ASSIGNMENT);

        // Check if the variable exists in any scope
        if (!variable.hasVariable(varName)) {
            // Variable is not declared, declare it in the current scope
            int32_t memoryLocation = variable.addVariable(varName);
            emit(Opcode::DECLARE_VARIABLE, token.line, memoryLocation);
        }

        // Get the memory location of the variable (assumes variable is now declared)
        int32_t memoryLocation = variable.getVariableMemoryLocation(varName);
        emit(Opcode::STORE_VARIABLE, token.line, memoryLocation);
}

void Parser::parseAnd()
{
    Token op = previous();
    parsePrecedence(static_cast<Precedence>(PREC_AND + 1));
    emit(Opcode::AND, op.line);
}

void Parser::parseOr()
{
    Token op = previous();
    parsePrecedence(static_cast<Precedence>(PREC_OR + 1));
    emit(Opcode::OR, op.line);
}

void Parser::parseLogical()
{
    Token op = previous();
    parsePrecedence(PREC_OR);
    if (op.type == TokenType::BANG) {
        emit(Opcode::NOT, op.line);
    }
}

void Parser::parseComparison()
{
    Token op = previous();
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

void Parser::parsePrintStatement()
{
    Token op = previous();
    parseExpression();
    emit(Opcode::PRINT, op.line);
}

void Parser::parseIfStatement()
{
    // Token op = previous();
    // consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    // parseExpression();
    // consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition");

    // size_t thenJump = emit(Opcode::JUMP_IF_FALSE, op.line,  0);

    // parseExpression();

    // if (match(TokenType::ELSE))
    // {
    //     size_t elseJump = emit(Opcode::JUMP, op.line, 0);
    //     int32_t jump = bytecode.size() - thenJump - 1;
    //     bytecode[thenJump].value = jump;
    //     parseExpression();
    //     int32_t jump = bytecode.size() - elseJump - 1;
    //     bytecode[elseJump].value = jump;
    // }
    // else
    // {
    //     int32_t jump = bytecode.size() - thenJump - 1;
    //     bytecode[thenJump].value = jump;
    // }
}

void Parser::parseWhileLoop()
{
    // Token op = previous();
    // consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
    // parseExpression();
    // consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");

    // size_t loopStart = bytecode.size();

    // size_t conditionJump = emit(Opcode::JUMP_IF_FALSE, op.line, 0);
    // parseStatement();
    // emit(Opcode::JUMP, op.line, (int32_t)(loopStart - bytecode.size() - 1));

    // int32_t jumpOffset = bytecode.size() - conditionJump - 1;
    // bytecode[conditionJump].value = jumpOffset;
}

void Parser::parseForLoop()
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

    // size_t bodyJump = emit(Opcode::JUMP_IF_FALSE, op.line, 0);
    // parseExpression();
    // emit(Opcode::JUMP, op.line, (int32_t)(conditionOffset - bytecode.size() - 1));
}

void Parser::parseMatchStatement()
{
    Token op = previous();
    parseExpression();
    emit(Opcode::PATTERN_MATCH, op.line);

    parseExpression();
    emit(Opcode::PATTERN_MATCH, op.line);
}

void Parser::parseFnDeclaration()
{
    consume(TokenType::FN, "Expected 'fn' keyword to start function definition");
    Token name = previous();
    // consume(TokenType::IDENTIFIER, "Expected function name");

    // Parse optional parameters
    std::vector<std::pair<std::string, std::optional<std::string>>> parameters;
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    if (peek().type != TokenType::RIGHT_PAREN) {
        do {
            Token paramName = peek();
            consume(TokenType::IDENTIFIER, "Expected parameter name");
            std::optional<std::string> defaultValue;
            if (match(TokenType::COLON)) {
                consume(TokenType::IDENTIFIER, "Expected type after ':' in parameter declaration");
                if (match(TokenType::EQUAL)) {
                    defaultValue = peek().lexeme;
                    advance();
                } else {
                    error("Expected '=' after type in parameter declaration with default value");
                }
            }
            parameters.push_back(std::make_pair(paramName.lexeme, defaultValue));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after function parameters");

    // Parse expected return type (optional)
    std::optional<std::string> returnType;
    if (match(TokenType::COLON)) {
        consume(TokenType::IDENTIFIER, "Expected return type after ':'");
        returnType = peek().lexeme;
        advance();
    }

    // Function body parsing logic goes here 
    parseBlock();
    //    consume(TokenType::LEFT_BRACE, "Expected '{' before function body");
    //    parseDeclaration();
    //    consume(TokenType::RIGHT_BRACE, "Expected '}' after function body");

    // Emit bytecode for function declaration
    emit(Opcode::DEFINE_FUNCTION, name.line, name.lexeme);
}

void Parser::parseFnCall() {
    Token name = previous();
  //consume(TokenType::IDENTIFIER, "Expected function name for call");
  consume(TokenType::LEFT_PAREN, "Expected '(' after function name in call");

  // Parse arguments (optional)
  std::vector<std::variant<int32_t, double, bool, std::string>> arguments;
  if (peek().type != TokenType::RIGHT_PAREN) {
    do {
      parseExpression();
      arguments.push_back(previous().lexeme);
    } while (match(TokenType::COMMA));
  }
  consume(TokenType::RIGHT_PAREN, "Expected ')' after function call arguments");
  // Emit bytecode for function call with arguments
  for (const auto &arg : arguments) {
    emit(Opcode::PUSH_ARGS, name.line, arg);
  }

  // Emit bytecode for function call with arguments
  emit(Opcode::INVOKE_FUNCTION, name.line, name.lexeme);
}

std::vector<Instruction> Parser::getBytecode() const
{
   // std::cout << "Getting the bytecode: " << std::endl;
    // Get the generated bytecode
    return bytecode;
}
