#include "pratt.hh"
#include "../debugger.hh"
#include "../instructions.hh"
#include "../scanner.hh"
#include <chrono>
#include <fstream>
#include <thread>

PrattParser::PrattParser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem)
    : scanner(scanner), typeSystem(typeSystem)
{
    tokens = scanner.scanTokens();
    parse();
}

Bytecode PrattParser::parse()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    scanner.current = 0;
    while (!isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) {
            ast.push_back(std::move(stmt));
        }
        if (hadError) {
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
    case TokenType::ELIF:
    case TokenType::ELSE:
        return &PrattParser::parseIf;
    case TokenType::WHILE:
        return &PrattParser::parseWhileLoop;
    case TokenType::FOR:
        return &PrattParser::parseForLoop;
    case TokenType::MATCH:
        return &PrattParser::parseMatchStatement;
    case TokenType::CONCURRENT:
        return &PrattParser::parseConcurrentStatement;
    case TokenType::PARALLEL:
        return &PrattParser::parseParallelStatement;
    case TokenType::SEMICOLON:
        return &PrattParser::createEmptyNode;
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
        return &PrattParser::parseTypes;
    case TokenType::IN:
    case TokenType::THIS:
    case TokenType::ENUM:
    case TokenType::ASYNC:
    case TokenType::AWAIT:
    case TokenType::CLASS:
        return &PrattParser::parseClassDeclaration;
    case TokenType::INTERFACE:
        return &PrattParser::parseInterface;
    case TokenType::MIXIN:
        return &PrattParser::parseMixin;
    case TokenType::UNSAFE:
        return &PrattParser::parseUnsafe;
    case TokenType::SUPER:
    case TokenType::IMPORT:
    case TokenType::RETURN:
    case TokenType::HANDLE:
    case TokenType::DEFAULT:
    case TokenType::ATTEMPT:
        return &PrattParser::createEmptyNode;
    case TokenType::COMMA:
    case TokenType::DOT:
    case TokenType::COLON:
    case TokenType::QUESTION:
    case TokenType::ARROW:
    case TokenType::LEFT_BRACKET:
    case TokenType::RIGHT_BRACKET:
    case TokenType::RIGHT_BRACE:
        return &PrattParser::createEmptyNode;
    case TokenType::UNDEFINED:
        return &PrattParser::parseUnexpected;
    default:
        return nullptr;
    }
}

void PrattParser::advance() {
    if (current < tokens.size() - 1) {
        current++;
    }
}

std::unique_ptr<ASTNode> PrattParser::createEmptyNode() {
    advance();
    return std::make_unique<ASTNode>();
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

void PrattParser::synchronize()
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

std::unique_ptr<ASTNode> PrattParser::parsePrimary()
{
    TokenType tokenType = peek().type;
    if (tokenType == TokenType::NUMBER || tokenType == TokenType::STRING) {
        return parseLiteral();
    } else if (tokenType == TokenType::TRUE || tokenType == TokenType::FALSE) {
        return parseBoolean();
    } else if (tokenType == TokenType::IDENTIFIER) {
        return parseIdentifier();
    } else if (tokenType == TokenType::LEFT_PAREN) {
        return parseParenthesis();
    } else if (tokenType == TokenType::MINUS || tokenType == TokenType::PLUS) {
        return parseUnary();
    } else if (isAtEnd()) {
        return parseEOF();
    } else {
        error("Unexpected token in primary expression");
        return nullptr;
    }
}

std::unique_ptr<ASTNode> PrattParser::parseExpression()
{
    if (check(TokenType::IF) || check(TokenType::ELIF) || check(TokenType::ELSE)) {
        return parseIfStatement();
    } else if (match(TokenType::WHILE)) {
        return parseWhileLoop();
    } else if (match(TokenType::FOR)) {
        return parseForLoop();
    } else {
        return parsePrecedence(PREC_ASSIGNMENT);
    }
}

std::unique_ptr<ASTNode> PrattParser::parseDeclaration()
{
    if (check(TokenType::VAR)) {
        return parseDecVariable();
    } else if (match(TokenType::FN)) {
        return parseFnDeclaration();
    } else if (match(TokenType::CLASS)) {
        return parseClassDeclaration();
    } else {
        return parseStatement();
    }
}

std::unique_ptr<ASTNode> PrattParser::parseStatement()
{
    if (check(TokenType::PRINT)) {
        return parsePrintStatement();
    } else if (check(TokenType::LEFT_BRACE)) {
        return parseBlock();
    } else if (check(TokenType::IF)) {
        return parseIfStatement();
    } else if (check(TokenType::ELIF)) {
        return parseIfStatement();
    } else if (check(TokenType::WHILE)) {
        return parseWhileLoop();
    } else if (check(TokenType::FOR)) {
        return parseForLoop();
    } else if (check(TokenType::MATCH)) {
        return parseMatchStatement();
    } else if (check(TokenType::IDENTIFIER) && peekNext().type == TokenType::EQUAL) {
        return parseAssignment();
    } else {
        return parseExpressionStatement();
    }
}

std::unique_ptr<ASTNode> PrattParser::parseExpressionStatement()
{
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return expr;
}

std::unique_ptr<ASTNode> PrattParser::parseParenthesis()
{
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
    auto expr = parseExpression(); // Parse the expression inside parentheses
    consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
    return expr;
}

std::unique_ptr<ASTNode> PrattParser::parseUnary()
{
    Token op = previous();
    advance(); // Consume the unary operator
    auto right = parsePrecedence(PREC_UNARY);
    return std::make_unique<UnaryNode>(op, std::move(right));
}

std::unique_ptr<ASTNode> PrattParser::parseBoolean()
{
    Token token = previous();
    bool value = (token.type == TokenType::TRUE);
    return std::make_unique<BooleanNode>(token, value);
}

std::unique_ptr<ASTNode> PrattParser::parseBinary()
{
    Token op = previous();
    auto left = parsePrecedence(static_cast<Precedence>(getTokenPrecedence(op.type) + 1));
    auto right = parsePrecedence(static_cast<Precedence>(getTokenPrecedence(op.type) + 1));
    return std::make_unique<BinaryNode>(op, std::move(left), std::move(right));
}

std::unique_ptr<ASTNode> PrattParser::parseLiteral()
{
    Token token = previous();
    TypePtr typePtr = std::make_shared<Type>(inferType(token));
    Value value = setValue(typePtr, token.lexeme);
    return std::make_unique<LiteralNode>(token, value);
}

std::unique_ptr<ASTNode> PrattParser::parseString()
{
    std::string str = previous().lexeme;
    TypePtr typePtr = std::make_shared<Type>(inferType(previous()));
    bool isInterpolated = (str.find('{') != std::string::npos) && (str.find('}') != std::string::npos);
    if (!isInterpolated) {
        return std::make_unique<StringNode>(previous(), str);
    }
    std::vector<std::unique_ptr<ASTNode>> parts;
    std::string current;
    bool inExpression = false;
    int bracketCount = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        if (c == '{' && !inExpression) {
            if (!current.empty()) {
                parts.push_back(std::make_unique<StringNode>(previous(), current));
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
                parts.push_back(parseExpression());
                current.clear();
                inExpression = false;
            } else {
                current += c;
            }
        } else if (c == '\\' && i + 1 < str.length()) {
            char nextChar = str[i + 1];
            if (nextChar == '{' || nextChar == '}') {
                i++;
            } else {
                current += c;
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(std::make_unique<StringLiteralNode>(previous(), current));
    }
    return std::make_unique<InterpolatedStringNode>(previous(), std::move(parts));
}

std::unique_ptr<ASTNode> PrattParser::parseIdentifier()
{
    Token nameToken = peek();
    Token nextToken = peekNext();
    if (nextToken.type == TokenType::EQUAL) {
        return parseAssignment();
    } else if (nextToken.type == TokenType::LEFT_PAREN) {
        return parseFnCall();
    } else {
        return parseLoadVariable();
    }
}

std::unique_ptr<ASTNode> PrattParser::parseDecVariable()
{
    Token name = peek();
    TypeTag type = TypeTag::Any;
    consume(TokenType::IDENTIFIER, "Expected variable name after 'var' token");
    if (check(TokenType::COLON)) {
        consume(TokenType::COLON, "Expected ':' after variable name");
        Token typeToken = peek();
        type = stringToType(typeToken.lexeme);
        advance();
    }
    consume(TokenType::EQUAL, "Expected '=' after type");
    auto initializer = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<VariableNode>(name, type, name.lexeme, true, std::move(initializer));
}

std::unique_ptr<ASTNode> PrattParser::parseLoadVariable()
{
    Token name = previous();
    return std::make_unique<VariableNode>(name, TypeTag::Any, name.lexeme, true, nullptr);
}

std::unique_ptr<ASTNode> PrattParser::parseBlock()
{
    std::vector<std::unique_ptr<Statement>> statements;
    enterScope();
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' at the end of a block");
    exitScope();
    return std::make_unique<BlockNode>(previous(), std::move(statements));
}

std::unique_ptr<ASTNode> PrattParser::parseAssignment()
{
    Token token = tokens[current];
    auto value = parsePrecedence(PREC_ASSIGNMENT);
    return std::make_unique<AssignmentNode>(token, std::make_unique<VariableNode>(token, TypeTag::Any, token.lexeme, true, nullptr), std::move(value));
}

std::unique_ptr<ASTNode> PrattParser::parseAnd()
{
    Token op = previous();
    auto left = parsePrecedence(static_cast<Precedence>(PREC_AND + 1));
    auto right = parsePrecedence(static_cast<Precedence>(PREC_AND + 1));
    return std::make_unique<BinaryNode>(op, std::move(left), std::move(right));
}

std::unique_ptr<ASTNode> PrattParser::parseOr()
{
    Token op = previous();
    auto left = parsePrecedence(static_cast<Precedence>(PREC_OR + 1));
    auto right = parsePrecedence(static_cast<Precedence>(PREC_OR + 1));
    return std::make_unique<BinaryNode>(op, std::move(left), std::move(right));
}

std::unique_ptr<ASTNode> PrattParser::parseLogical()
{
    Token op = previous();
    auto right = parsePrecedence(PREC_OR);
    return std::make_unique<UnaryNode>(op, std::move(right));
}

std::unique_ptr<ASTNode> PrattParser::parseComparison()
{
    Token op = previous();
    auto left = parsePrecedence(static_cast<Precedence>(getTokenPrecedence(op.type) + 1));
    auto right = parsePrecedence(static_cast<Precedence>(getTokenPrecedence(op.type) + 1));
    return std::make_unique<BinaryNode>(op, std::move(left), std::move(right));
}

std::unique_ptr<ASTNode> PrattParser::parsePrintStatement()
{
    Token op = peek();
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after print function.");
    return std::make_unique<PrintNode>(op, std::move(expr));
}

std::unique_ptr<ASTNode> PrattParser::parseIfStatement()
{
    Token ifToken = previous();
    auto condition = parseExpression();
    auto thenBranch = parseBlock();
    std::optional<std::unique_ptr<Statement>> elseBranch = std::nullopt;

    // Handle multiple elif statements
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> elifBranches;
    while (match(TokenType::ELIF)) {
        auto elifCondition = parseExpression();
        auto elifBranch = parseBlock();
        elifBranches.emplace_back(std::move(elifCondition), std::move(elifBranch));
    }

    // Handle else statement
    if (match(TokenType::ELSE)) {
        elseBranch = parseBlock();
    }

    return std::make_unique<ConditionalNode>(ifToken, std::move(condition), std::move(thenBranch), std::move(elifBranches), std::move(elseBranch));
}

std::unique_ptr<ASTNode> PrattParser::parseWhileLoop()
{
    Token whileToken = previous();
    auto condition = parseExpression();
    auto body = parseBlock();
    return std::make_unique<WhileNode>(whileToken, std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> PrattParser::parseForLoop()
{
    Token forToken = previous();
    auto initializer = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after loop initializer");
    auto condition = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after loop condition");
    auto increment = parseExpression();
    auto body = parseBlock();
    return std::make_unique<ForNode>(forToken, std::move(initializer), std::move(condition), std::move(increment), std::move(body));
}

std::unique_ptr<ASTNode> PrattParser::parseMatchStatement()
{
    Token matchToken = previous();
    auto matchExpression = parseExpression();
    consume(TokenType::LEFT_BRACE, "Expected '{' after match expression.");
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> matchCases;
    std::optional<std::unique_ptr<Statement>> defaultCase = std::nullopt;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match(TokenType::DEFAULT)) {
            defaultCase = parseBlock();
        } else {
            auto caseExpression = parseExpression();
            auto caseBody = parseBlock();
            matchCases.emplace_back(std::move(caseExpression), std::move(caseBody));
        }
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after match cases.");
    return std::make_unique<MatchNode>(matchToken, std::move(matchExpression), std::move(matchCases), std::move(defaultCase));
}

std::unique_ptr<ASTNode> PrattParser::parseConcurrentStatement()
{
    Token concurrentToken = previous();
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'concurrent'");
    std::vector<std::unique_ptr<Statement>> branches;
    while (!check(TokenType::RIGHT_PAREN)) {
        branches.push_back(parseExpressionStatement());
        if (match(TokenType::COMMA)) {
            continue;
        }
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after concurrent arguments");
    auto body = parseBlock();
    branches.push_back(std::move(body));
    return std::make_unique<ConcurrentNode>(concurrentToken, std::move(branches));
}

std::unique_ptr<ASTNode> PrattParser::parseParallelStatement()
{
    Token parallelToken = previous();
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'parallel'");
    std::vector<std::unique_ptr<Statement>> branches;
    while (!check(TokenType::RIGHT_PAREN)) {
        branches.push_back(parseExpressionStatement());
        if (match(TokenType::COMMA)) {
            continue;
        }
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parallel arguments");
    auto body = parseBlock();
    branches.push_back(std::move(body));
    return std::make_unique<ParallelNode>(parallelToken, std::move(branches));
}

std::unique_ptr<ASTNode> PrattParser::parseFnDeclaration()
{
    Token fnToken = previous();
    consume(TokenType::IDENTIFIER, "Expected function name");
    std::string name = previous().lexeme;
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    std::vector<Parameter> parameters;
    while (!check(TokenType::RIGHT_PAREN)) {
        Token paramName = peek();
        consume(TokenType::IDENTIFIER, "Expected parameter name");
        Type paramType = TypeTag::Any;
        if (match(TokenType::COLON)) {
            Token typeToken = peek();
            paramType = stringToType(typeToken.lexeme);
            advance();
        }
        parameters.emplace_back(paramName.lexeme, paramType);
        if (match(TokenType::COMMA)) {
            continue;
        }
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
    Type returnType = TypeTag::Nil;
    if (match(TokenType::COLON)) {
        Token typeToken = peek();
        returnType = stringToType(typeToken.lexeme);
        advance();
    }
    auto body = parseBlock();
    return std::make_unique<FunctionNode>(fnToken, name, returnType, std::move(parameters), std::move(body));
}

std::unique_ptr<ASTNode> PrattParser::parseFnCall()
{
    Token fnToken = previous();
    std::string name = fnToken.lexeme;
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
    std::vector<std::unique_ptr<Expression>> arguments;
    while (!check(TokenType::RIGHT_PAREN)) {
        arguments.push_back(parseExpression());
        if (match(TokenType::COMMA)) {
            continue;
        }
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
    return std::make_unique<CallNode>(fnToken, name, std::move(arguments));
}

std::unique_ptr<ASTNode> PrattParser::parseImport()
{
    Token importToken = previous();
    consume(TokenType::IDENTIFIER, "Expected module name");
    std::string moduleName = previous().lexeme;
    std::optional<std::string> alias = std::nullopt;
    if (match(TokenType::AS)) {
        consume(TokenType::IDENTIFIER, "Expected alias name");
        alias = previous().lexeme;
    }
    consume(TokenType::SEMICOLON, "Expected ';' after import statement");
    return std::make_unique<ImportNode>(importToken, moduleName, alias);
}

std::unique_ptr<ASTNode> PrattParser::parseModules()
{
    Token moduleToken = previous();
    consume(TokenType::IDENTIFIER, "Expected module name");
    std::string moduleName = previous().lexeme;
    consume(TokenType::LEFT_BRACE, "Expected '{' after module name");
    std::vector<std::unique_ptr<Statement>> declarations;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        declarations.push_back(parseDeclaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after module declarations");
    return std::make_unique<ModuleNode>(moduleToken, moduleName, std::move(declarations));
}

std::unique_ptr<ASTNode> PrattParser::parseTypes()
{
    Token typeToken = previous();
    TypeTag type = stringToType(typeToken.lexeme);
    return std::make_unique<TypeNode>(typeToken, type);
}

std::unique_ptr<ASTNode> PrattParser::parseClassDeclaration()
{
    Token classToken = previous();
    consume(TokenType::IDENTIFIER, "Expected class name");
    std::string className = previous().lexeme;
    std::optional<std::string> baseClass = std::nullopt;
    if (match(TokenType::COLON)) {
        consume(TokenType::IDENTIFIER, "Expected base class name");
        baseClass = previous().lexeme;
    }
    consume(TokenType::LEFT_BRACE, "Expected '{' after class declaration");
    std::vector<std::unique_ptr<Statement>> members;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        members.push_back(parseDeclaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after class members");
    return std::make_unique<ClassNode>(classToken, className, baseClass, std::move(members));
}

std::unique_ptr<ASTNode> PrattParser::parseInterface() {
    Token interfaceToken = previous();
    consume(TokenType::IDENTIFIER, "Expected interface name");
    std::string name = previous().lexeme;
    consume(TokenType::LEFT_BRACE, "Expected '{' after interface name");
    std::vector<std::unique_ptr<Statement>> methods;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        methods.push_back(parseDeclaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after interface methods");
    return std::make_unique<InterfaceNode>(interfaceToken, name, std::move(methods));
}

std::unique_ptr<ASTNode> PrattParser::parseMixin() {
    Token mixinToken = previous();
    consume(TokenType::IDENTIFIER, "Expected mixin name");
    std::string name = previous().lexeme;
    consume(TokenType::LEFT_BRACE, "Expected '{' after mixin name");
    std::vector<std::unique_ptr<Statement>> methods;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        methods.push_back(parseDeclaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after mixin methods");
    return std::make_unique<MixinNode>(mixinToken, name, std::move(methods));
}

std::unique_ptr<ASTNode> PrattParser::parseUnsafe() {
    Token unsafeToken = previous();
    consume(TokenType::LEFT_BRACE, "Expected '{' after 'unsafe'");
    std::vector<std::unique_ptr<Statement>> body;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        body.push_back(parseStatement());
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after unsafe block");
    return std::make_unique<UnsafeNode>(unsafeToken, std::move(body));
}

std::unique_ptr<ASTNode> PrattParser::parseInterpolatedString() {
    Token token = previous();
    std::string str = token.lexeme;
    std::vector<std::unique_ptr<ASTNode>> parts;
    std::string current;
    bool inExpression = false;
    int bracketCount = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        if (c == '{' && !inExpression) {
            if (!current.empty()) {
                parts.push_back(std::make_unique<StringLiteralNode>(token, Type{TypeTag::String}, current));
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
                parts.push_back(parseExpression());
                current.clear();
                inExpression = false;
            } else {
                current += c;
            }
        } else if (c == '\\' && i + 1 < str.length()) {
            char nextChar = str[i + 1];
            if (nextChar == '{' || nextChar == '}') {
                i++;
            } else {
                current += c;
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(std::make_unique<StringLiteralNode>(token, Type{TypeTag::String}, current));
    }
    return std::make_unique<InterpolatedStringNode>(token, std::move(parts));
}
