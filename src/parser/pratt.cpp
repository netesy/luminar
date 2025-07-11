#include "pratt.hh"
#include "../visitors/bytecode_generator.hh"
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>

PrattParser::PrattParser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem)
    : scanner(scanner)
    , variable(typeSystem)
    , typeSystem(typeSystem)
{
    tokens = scanner.scanTokens();
    parse();
}

Bytecode PrattParser::parse()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    current = 0;
    while (!isAtEnd()) {
        auto stmt = parseDeclaration();
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

    // Generate bytecode from the AST
    generateBytecode();
    return bytecode;
}

// Helper function to create a Value from a variant
struct ValueCreator {
    TypeSystem* typeSystem;

    ValuePtr operator()(std::monostate) const {
        auto value = std::make_shared<Value>();
        value->type = typeSystem->NIL_TYPE;
        value->data = std::monostate{};
        return value;
    }

    ValuePtr operator()(int val) const {
        auto value = std::make_shared<Value>();
        value->type = typeSystem->INT_TYPE;
        value->data = static_cast<int32_t>(val);
        return value;
    }

    ValuePtr operator()(float val) const {
        auto value = std::make_shared<Value>();
        value->type = typeSystem->FLOAT64_TYPE;
        value->data = static_cast<double>(val);
        return value;
    }

    ValuePtr operator()(const std::string& val) const {
        auto value = std::make_shared<Value>();
        value->type = typeSystem->STRING_TYPE;
        value->data = val;
        return value;
    }

    ValuePtr operator()(const Value& val) const {
        return std::make_shared<Value>(val);
    }
};



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

// Core of the Pratt parser: parse a prefix expression then repeatedly parse infix expressions
std::unique_ptr<ASTNode> PrattParser::parseExpression(Precedence precedence) {
    // Get parsing function for the current token's type
    PrefixParseFn prefixFn = getPrefixParseFn(peek().type);
    if (!prefixFn) {
        error("Expected expression, but got " + peek().lexeme);
        return nullptr;
    }

    // Consume the token and parse the prefix expression
    Token prefixToken = advance();
    auto leftExpr = (this->*prefixFn)(std::move(prefixToken));

    // While the next token has higher precedence, parse infix expressions
    while (leftExpr && precedence < getTokenPrecedence(peek().type) && !isAtEnd()) {
        InfixParseFn infixFn = getInfixParseFn(peek().type);
        if (!infixFn) {
            break;
        }

        Token infixToken = advance();
        leftExpr = (this->*infixFn)(std::move(leftExpr), std::move(infixToken));
    }

    return leftExpr;
}

// Methods to retrieve parsing functions
PrefixParseFn PrattParser::getPrefixParseFn(TokenType type) {
    switch (type) {
    case TokenType::NUMBER:
    case TokenType::STRING:
        return &PrattParser::parseLiteral;
    case TokenType::TRUE:
    case TokenType::FALSE:
        return &PrattParser::parseBoolean;
    // case TokenType::IDENTIFIER:
    //     return &PrattParser::parseIdentifier;
    case TokenType::LEFT_PAREN:
        return &PrattParser::parseGrouping;
    case TokenType::MINUS:
    case TokenType::BANG:
        return &PrattParser::parseUnary;
    // case TokenType::VAR:
    //     return &PrattParser::parseDecVariable;
    // case TokenType::FN:
    //     return &PrattParser::parseFnDeclaration;
    // Add more prefix parse functions as needed
    default:
        return nullptr;
    }
}

InfixParseFn PrattParser::getInfixParseFn(TokenType type) {
    switch (type) {
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::MODULUS:
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
        return &PrattParser::parseBinaryOp;
    case TokenType::AND:
        return &PrattParser::parseAndOp;
    case TokenType::OR:
        return &PrattParser::parseOrOp;
    // case TokenType::EQUAL:
    // case TokenType::PLUS_EQUAL:
    // case TokenType::MINUS_EQUAL:
    //     return &PrattParser::parseAssignment;
    // case TokenType::LEFT_PAREN:
    //     return &PrattParser::parseCall;
    // Add more infix parse functions as needed
    default:
        return nullptr;
    }
}

// Token handling utilities
Token PrattParser::advance() {
    Token current_token = peek();
    if (!isAtEnd()) {
        current++;
    }
    return current_token;
}

Token PrattParser::peek() {
    return tokens[current];
}

Token PrattParser::peekNext() {
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

bool PrattParser::isAtEnd() {
    return peek().type == TokenType::EOF_TOKEN;
}

Token PrattParser::previous() {
    if (current > 0) {
        return tokens[current - 1];
    }
    return tokens[0];
}

bool PrattParser::check(TokenType type) {
    return !isAtEnd() && peek().type == type;
}

bool PrattParser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void PrattParser::consume(TokenType type, const std::string &message) {
    if (check(type)) {
        advance();
    } else {
        error(message + ", found '" + peek().lexeme + "'");
    }
}

// Error handling
void PrattParser::error(const std::string &message) {
    hadError = true;

    // Create a SourceLocation for the error
    SourceLocation loc(peek().line,peek().column, peek().filename);

    // Attach the error to the current node's metadata
    if (currentNode) {
        currentNode->getOrCreateMetadata().addError(
            NodeMetadata::ErrorDetail::Severity::Error,
            message,
            std::nullopt, // No suggestion
            loc // Source location
            );
    } else {
        // If no current node, log the error directly
        //Debugger::error(message, peek(), InterpretationStage::PARSING, scanner.getSource());
    }
}

void PrattParser::synchronize() {
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

// Token precedence - Order matches packrat parser's expression hierarchy
Precedence PrattParser::getTokenPrecedence(TokenType type) {
    switch (type) {
    case TokenType::OR:
        return PREC_OR;
    case TokenType::AND:
        return PREC_AND;
    case TokenType::EQUAL_EQUAL:
    case TokenType::BANG_EQUAL:
        return PREC_EQUALITY;
    case TokenType::LESS:
    case TokenType::LESS_EQUAL:
    case TokenType::GREATER:
    case TokenType::GREATER_EQUAL:
        return PREC_COMPARISON;
    case TokenType::DOT_DOT:
        return PREC_RANGE;
    case TokenType::PLUS:
    case TokenType::MINUS:
        return PREC_TERM;
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::MODULUS:
        return PREC_FACTOR;
    case TokenType::BANG:
        return PREC_UNARY;
    case TokenType::LEFT_PAREN:  // Function calls
    case TokenType::DOT:         // Method calls and property access
    case TokenType::LEFT_BRACKET: // Subscript/indexing
        return PREC_CALL;
    case TokenType::EQUAL:       // Assignment
    case TokenType::PLUS_EQUAL:  // Compound assignment
    case TokenType::MINUS_EQUAL: // Compound assignment
        return PREC_ASSIGNMENT;
    default:
        return PREC_NONE;
    }
}

// Bytecode emission
Instruction PrattParser::emit(Opcode opcode, uint32_t lineNumber) {
    Instruction instruction(opcode, lineNumber);
    instruction.debug();
    bytecode.push_back(instruction);
    return instruction;
}

Instruction PrattParser::emit(Opcode opcode, uint32_t lineNumber, Value &&value) {
    ValuePtr valuePtr = std::make_shared<Value>(std::move(value));
    Instruction instruction(opcode, lineNumber, valuePtr);
    instruction.debug();
    bytecode.push_back(instruction);
    return instruction;
}

// Prefix parse functions
std::unique_ptr<ASTNode> PrattParser::parseLiteral(Token token) {
    return parsePrimary();
}

std::unique_ptr<ASTNode> PrattParser::parseBoolean(Token token) {
    TypePtr typePtr = std::make_shared<Type>(inferType(token));
    Value value = setValue(typePtr, token.lexeme);
    auto node = std::make_unique<BooleanNode>(SourceLocation(token.line,
                                                             token.column,
                                                             token.filename),
                                              Type{TypeTag::Bool},
                                              value);
    currentNode = node.get(); // Set currentNode
    return node;
}

// Infix parse functions
std::unique_ptr<ASTNode> PrattParser::parseBinaryOp(std::unique_ptr<ASTNode> left, Token token) {
    Precedence precedence = getTokenPrecedence(token.type);
    auto right = parseExpression(static_cast<Precedence>(precedence + 1));

    // Safe casting with error handling
    auto leftExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(left.release()));
    auto rightExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(right.release()));

    if (!leftExpr || !rightExpr) {
        error("Invalid operands for binary operation");
        return createEmptyNode();
    }

    auto node = std::make_unique<BinaryNode>(
        SourceLocation(token.line, token.column),
        Type{TypeTag::Any},
        token.lexeme,
        std::move(leftExpr),
        std::move(rightExpr)
    );
    currentNode = node.get();
    return node;
}


std::unique_ptr<ASTNode> PrattParser::parseAndOp(std::unique_ptr<ASTNode> left, Token token) {
    // AND has right associativity, so we use the same precedence
    auto right = parseExpression(PREC_AND);

    auto leftExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(left.release()));
    auto rightExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(right.release()));

    if (!leftExpr || !rightExpr) {
        error("Invalid operands for AND operation");
        return createEmptyNode();
    }

    auto node = std::make_unique<BinaryNode>(
        SourceLocation(token.line, token.column),
        Type{TypeTag::Bool},
        token.lexeme,
        std::move(leftExpr),
        std::move(rightExpr)
        );
    currentNode = node.get();
    return node;
}

std::unique_ptr<ASTNode> PrattParser::parseOrOp(std::unique_ptr<ASTNode> left, Token token) {
    // OR has right associativity, so we use the same precedence
    auto right = parseExpression(PREC_OR);

    auto leftExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(left.release()));
    auto rightExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(right.release()));

    if (!leftExpr || !rightExpr) {
        error("Invalid operands for OR operation");
        return createEmptyNode();
    }

    auto node = std::make_unique<BinaryNode>(
        SourceLocation(token.line, token.column),
        Type{TypeTag::Bool},
        token.lexeme,
        std::move(leftExpr),
        std::move(rightExpr)
        );
    currentNode = node.get();
    return node;
}

std::unique_ptr<ASTNode> PrattParser::parseUnary(Token token) {
    // Parse the operand with unary precedence
    auto right = parseExpression(PREC_UNARY);
   auto node  = std::make_unique<UnaryNode>(token, std::move(right));
    currentNode = node.get(); // Set currentNode
    return node;
}

std::unique_ptr<ASTNode> PrattParser::parseGrouping(Token token) {
    // Parse the expression inside the parentheses
    auto expr = parseExpression(PREC_NONE);
    consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
    return expr;
}

// Parse a primary expression (first in precedence)
std::unique_ptr<ASTNode> PrattParser::parsePrimary() {
    if (match(TokenType::FALSE)) {
        Token token = previous();
        TypePtr typePtr = std::make_shared<Type>(inferType(token));
        Value value = setValue(typePtr, token.lexeme);
        auto node = std::make_unique<BooleanNode>(
            SourceLocation(token.line, token.column, token.filename),
            Type{TypeTag::Bool},
            value
            );
        currentNode = node.get();
        return node;
    }

    if (match(TokenType::TRUE)) {
        Token token = previous();
        TypePtr typePtr = std::make_shared<Type>(inferType(token));
        Value value = setValue(typePtr, token.lexeme);
        auto node = std::make_unique<BooleanNode>(
            SourceLocation(token.line, token.column, token.filename),
            Type{TypeTag::Bool},
            value
            );
        currentNode = node.get();
        return node;
    }

    if (match(TokenType::NIL_TYPE)) {
        auto node = std::make_unique<NilNode>(SourceLocation(previous().line, previous().column));
        currentNode = node.get();
        return node;
    }

    if (match(TokenType::NUMBER)) {
        Token token = previous();
        TypePtr typePtr = std::make_shared<Type>(inferType(token));
        Value value = setValue(typePtr, token.lexeme);
        auto node = std::make_unique<NumberNode>(
            SourceLocation(token.line, token.column),
            Type{inferType(token)},
            value
            );
        currentNode = node.get();
        return node;
    }

    if (match(TokenType::STRING)) {
        Token token = previous();
        auto node = std::make_unique<StringLiteralNode>(
            SourceLocation(token.line, token.column),
            Type{TypeTag::String},
            Value{std::make_shared<Type>(TypeTag::String), token.lexeme}
            );
        currentNode = node.get();
        return node;
    }

    if (match(TokenType::IDENTIFIER)) {
        Token token = previous();
        auto node = std::make_unique<VariableNode>(
            SourceLocation(token.line, token.column),
            Type{TypeTag::Any},
            token.lexeme
            );
        currentNode = node.get();
        return node;
    }

    if (match(TokenType::LEFT_PAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        auto node = std::make_unique<GroupingNode>(std::move(expr));
        currentNode = node.get();
        return node;
    }

    error("Expected expression.");
    return nullptr;
}

// General expression parsing
std::unique_ptr<ASTNode> PrattParser::parseExpression() {
    auto node = parseExpression(PREC_ASSIGNMENT);
    // Don't reset currentNode here as it's handled by the calling function
    return node;
}

// Parse logical expressions
std::unique_ptr<ASTNode> PrattParser::parseLogical() {
    auto left = parseComparison();

    while (match(TokenType::AND) || match(TokenType::OR)) {
        Token op = previous();
        auto right = parseComparison();

        // Safely cast and transfer ownership
        auto leftExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(left.release()));
        auto rightExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(right.release()));

        if (!leftExpr || !rightExpr) {
            error("Invalid operands for logical operation");
            return createEmptyNode();
        }

        left = std::move(std::make_unique<BinaryNode>(
            SourceLocation(op.line, op.column),
            Type{inferType(op)},
            op.lexeme,
            std::move(leftExpr),
            std::move(rightExpr)
            ));
    }

    return left;
}

// Parse string literals
std::unique_ptr<ASTNode> PrattParser::parseString() {
    Token token = previous();
    auto node = std::make_unique<StringLiteralNode>(
        SourceLocation(token.line, token.column, token.filename),
        Type{TypeTag::String},
        Value{std::make_shared<Type>(TypeTag::String), token.lexeme}
        );
    currentNode = node.get();
    return node;
}


// Statement parsing methods
std::unique_ptr<ASTNode> PrattParser::parseDeclaration() {
    // if (match(TokenType::VAR)) {
    //     return parseDecVariable(previous());
    // } else if (match(TokenType::FN)) {
    //     return parseFnDeclaration(previous());
    // } else if (match(TokenType::CLASS)) {
    //     return parseClassDeclaration(previous());
    // } else {
        return parseStatement();
    // }
}

std::unique_ptr<ASTNode> PrattParser::parseStatement() {
    // if (match(TokenType::PRINT)) {
    //     return parsePrintStatement(previous());
    // } else
    // if (match(TokenType::LEFT_BRACE)) {
    //     return parseBlock(previous());
    // } else if (match(TokenType::IF)) {
    //     return parseIfStatement(previous());
    // } else if (match(TokenType::WHILE)) {
    //     return parseWhileLoop(previous());
    // } else if (match(TokenType::FOR)) {
    //     return parseForLoop(previous());
    // } else if (match(TokenType::MATCH)) {
    //     return parseMatchStatement(previous());
    // } else {
        return parseExpressionStatement();
    // }
}

std::unique_ptr<ASTNode> PrattParser::parseExpressionStatement() {
    auto expr = parseExpression(PREC_NONE);
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return expr;
}

// // Parse code blocks
// std::unique_ptr<ASTNode> PrattParser::parseBlock() {
//     std::vector<std::unique_ptr<Statement>> statements;
//     enterScope();

//     while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
//         auto stmt = parseStatement();

//         if (stmt) {
//             if (auto* stmtPtr = dynamic_cast<Statement*>(stmt.get())) {
//                 stmt.release();
//                 statements.push_back(std::unique_ptr<Statement>(stmtPtr));
//             } else {
//                 error("Only statements are allowed in a block");
//             }
//         }
//     }

//     consume(TokenType::RIGHT_BRACE, "Expected '}' at the end of a block");
//     exitScope();

//     auto node = std::make_unique<BlockNode>(
//         SourceLocation(peek().line, peek().column),
//         std::move(statements)
//         );
//     currentNode = node.get();
//     return node;
// }

// std::unique_ptr<ASTNode> PrattParser::parseBlock(Token token) {
//     std::vector<std::unique_ptr<Statement>> statements;
//     enterScope();

//     while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
//         auto stmt = parseStatement();

//         if (stmt) {
//             if (auto* stmtPtr = dynamic_cast<Statement*>(stmt.get())) {
//                 statements.emplace_back(std::unique_ptr<Statement>(dynamic_cast<Statement*>(stmt.release())));
//             } else {
//                 error("Only statements are allowed in a block");
//             }
//         }
//     }

//     consume(TokenType::RIGHT_BRACE, "Expected '}' at the end of a block");
//     exitScope();

//     auto node = std::make_unique<BlockNode>(
//         SourceLocation(token.line, token.column, token.filename),
//         std::move(statements)
//         );
//     currentNode = node.get();
//     return node;
// }

// //Loops and Conditionals
// std::unique_ptr<ASTNode> PrattParser::parseIfStatement(Token token) {
//     auto condition = parseExpression(PREC_NONE);
//     auto thenBranch = parseBlock(advance());

//     std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> elifBranches;
//     std::optional<std::unique_ptr<Statement>> elseBranch = std::nullopt;

//     while (match(TokenType::ELIF)) {
//         auto elifCondition = parseExpression(PREC_NONE);
//         auto elifBlock = parseBlock(advance());

//         elifBranches.emplace_back(
//             std::unique_ptr<Expression>(dynamic_cast<Expression*>(elifCondition.release())),
//             std::unique_ptr<Statement>(dynamic_cast<Statement*>(elifBlock.release()))
//         );
//     }

//     if (match(TokenType::ELSE)) {
//         auto elseNode = parseBlock(advance());
//         elseBranch = std::unique_ptr<Statement>(dynamic_cast<Statement*>(elseNode.release()));
//     }

//     auto conditionExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(condition.release()));
//     auto thenStmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(thenBranch.release()));

//     if (!conditionExpr || !thenStmt) {
//         error("Invalid if condition or then block");
//         return nullptr;
//     }

//     auto node = std::make_unique<ConditionalNode>(
//         SourceLocation(token.line, token.column, token.filename),
//         std::move(conditionExpr),
//         std::move(thenStmt),
//         std::move(elifBranches),
//         elseBranch ? std::move(*elseBranch) : nullptr
//     );
//     currentNode = node.get();
//     return node;
// }

// std::unique_ptr<ASTNode> PrattParser::parseWhileLoop(Token token) {
//     consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
//     auto condition = parseExpression(PREC_NONE);
//     consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");

//     auto body = parseStatement();

//     auto condExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(condition.release()));
//     auto bodyStmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(body.release()));

//     if (!condExpr || !bodyStmt) {
//         error("Invalid while condition or body");
//         return createEmptyNode();
//     }

//     auto node = std::make_unique<WhileNode>(
//         SourceLocation(token.line, token.column, token.filename),
//         std::move(condExpr),
//         std::move(bodyStmt)
//         );

//     currentNode = node.get();
//     return node;
// }

// std::unique_ptr<ASTNode> PrattParser::parseForLoop(Token token) {
//     consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'");

//     // Initializer
//     auto initializer = parseExpression(PREC_NONE);
//     consume(TokenType::SEMICOLON, "Expected ';' after loop initializer");

//     // Condition
//     auto condition = parseExpression(PREC_NONE);
//     consume(TokenType::SEMICOLON, "Expected ';' after loop condition");

//     // Increment
//     auto increment = parseExpression(PREC_NONE);
//     consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses");

//     // Body
//     auto body = parseStatement();

//     auto condExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(condition.release()));
//     auto bodyStmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(body.release()));

//     if (!condExpr || !bodyStmt) {
//         error("Invalid for condition or body");
//         return createEmptyNode();
//     }

//     auto node = std::make_unique<ForNode>(
//         SourceLocation(token.line, token.column, token.filename),
//         std::move(initializer),
//         std::move(condExpr),
//         std::move(increment),
//         std::move(bodyStmt)
//         );

//     currentNode = node.get();
//     return node;
// }

// // Parse if expressions
// std::unique_ptr<ASTNode> PrattParser::parseIf() {
//     auto condition = parseExpression();

//     consume(TokenType::LEFT_BRACE, "Expect '{' after if condition.");
//     auto thenBranch = parseBlock();

//     std::optional<std::unique_ptr<Statement>> elseBranch = std::nullopt;
//     if (match(TokenType::ELSE)) {
//         if (match(TokenType::IF)) {
//             auto elseIfNode = parseIf();
//             if (auto* stmtPtr = dynamic_cast<Statement*>(elseIfNode.get())) {
//                 elseIfNode.release();
//                 elseBranch = std::unique_ptr<Statement>(stmtPtr);
//             }
//         } else {
//             consume(TokenType::LEFT_BRACE, "Expect '{' after else.");
//             auto elseNode = parseBlock();
//             if (auto* stmtPtr = dynamic_cast<Statement*>(elseNode.get())) {
//                 elseNode.release();
//                 elseBranch = std::unique_ptr<Statement>(stmtPtr);
//             }
//         }
//     }

//     // Safely cast and transfer ownership
//     auto conditionExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(condition.release()));
//     auto thenStmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(thenBranch.release()));

//     if (!conditionExpr || !thenStmt) {
//         error("Invalid condition or then branch in if statement");
//         return createEmptyNode();
//     }

//     auto node = std::make_unique<ConditionalNode>(
//         SourceLocation(previous().line, previous().column),
//         std::move(conditionExpr),
//         std::move(thenStmt),
//         std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>>{},
//         std::move(elseBranch)
//         );
//     currentNode = node.get();
//     return node;
// }

// std::unique_ptr<ASTNode> PrattParser::parseMatchStatement(Token token) {
//     auto matchExpr = parseExpression(PREC_NONE);
//     consume(TokenType::LEFT_BRACE, "Expected '{' after match expression");

//     std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> matchCases;
//     std::optional<std::unique_ptr<Statement>> defaultCase = std::nullopt;

//     while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
//         if (match(TokenType::DEFAULT)) {
//             consume(TokenType::LEFT_BRACE, "Expected '{' after default");
//             auto defNode = parseBlock(previous());
//             auto defStmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(defNode.release()));
//             if (!defStmt) {
//                 error("Default case must be a statement");
//                 return createEmptyNode();
//             }
//             defaultCase = std::move(defStmt);
//         } else {
//             auto caseExpr = parseExpression(PREC_NONE);
//             consume(TokenType::LEFT_BRACE, "Expected '{' after case expression");
//             auto caseBody = parseBlock(previous());

//             auto caseExprPtr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(caseExpr.release()));
//             auto caseStmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(caseBody.release()));

//             if (!caseExprPtr || !caseStmt) {
//                 error("Invalid case expression or body");
//                 return createEmptyNode();
//             }

//             matchCases.emplace_back(std::move(caseExprPtr), std::move(caseStmt));
//         }
//     }

//     consume(TokenType::RIGHT_BRACE, "Expected '}' after match cases");

//     auto exprPtr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(matchExpr.release()));
//     if (!exprPtr) {
//         error("Match expression must be an expression");
//         return createEmptyNode();
//     }

//     auto node = std::make_unique<MatchNode>(
//         SourceLocation(token.line, token.column, token.filename),
//         std::move(exprPtr),
//         std::move(matchCases),
//         std::move(defaultCase)
//         );

//     currentNode = node.get();
//     return node;
// }


// std::unique_ptr<ASTNode> PrattParser::parseRangeExpression() {
//     auto left = parseComparison();

//     if (match(TokenType::DOT_DOT)) {
//         Token op = previous();
//         auto right = parseComparison();
//         std::unique_ptr<ASTNode> step = nullptr;

//         // Check for step value (..<step>)
//         if (match(TokenType::LESS)) {
//             step = parseComparison();
//         }

//         if (step) {
//             // Range with step
//             auto leftExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(left.release()));
//             auto stepExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(step.release()));

//             if (!leftExpr || !stepExpr) {
//                 error("Invalid operands for range operation");
//                 return createEmptyNode();
//             }

//             return std::make_unique<BinaryNode>(
//                 SourceLocation(op.line, op.column),
//                 Type{TypeTag::Any},
//                 "..<",
//                 std::move(leftExpr),
//                 std::move(stepExpr)
//                 );
//         } else {
//             // Simple range without step
//             auto leftExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(left.release()));
//             auto rightExpr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(right.release()));

//             if (!leftExpr || !rightExpr) {
//                 error("Invalid operands for range operation");
//                 return createEmptyNode();
//             }

//             return std::make_unique<BinaryNode>(
//                 SourceLocation(op.line, op.column),
//                 Type{TypeTag::Any},
//                 "..",
//                 std::move(leftExpr),
//                 std::move(rightExpr)
//                 );
//         }
//     }

//     return left;
// }

// std::unique_ptr<ASTNode> PrattParser::parseDecVariable(Token token) {
//     // // Expect variable name (identifier)
//     consume(TokenType::IDENTIFIER, "Expected variable name after 'var'");
//     Token name = previous();

//     // Check for type annotation
//     TypeTag type = TypeTag::Any;
//     if (match(TokenType::COLON)) {
//         Token typeToken = advance();
//         type = stringToType(typeToken.lexeme);
//     }

//     std::optional<std::unique_ptr<Expression>> initializer;
//     if (match(TokenType::EQUAL)) {
//         auto init = parseExpression(PREC_NONE);
//         if (auto* expr = dynamic_cast<Expression*>(init.get())) {
//             initializer = std::unique_ptr<Expression>(expr);
//             init.release();
//         } else {
//             error("Invalid initializer expression");
//         }
//     }

//     consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

//     auto node = std::make_unique<VariableNode>(SourceLocation(name.line, name.column),
//                                                Type{type},
//                                                name.lexeme,
//                                                true, // isMutable
//                                                std::move(initializer));
//     currentNode = node.get(); // Set currentNode
//     return node;
// }

// std::unique_ptr<ASTNode> PrattParser::parseIdentifier(Token token) {
//     // If next token is '(', we're calling a function
//     if (check(TokenType::LEFT_PAREN)) {
//         advance(); // Consume '('
//         std::vector<std::unique_ptr<ASTNode>> arguments;
//         if (!check(TokenType::RIGHT_PAREN)) {
//             do {
//                 arguments.push_back(parseExpression(PREC_NONE));
//             } while (match(TokenType::COMMA));
//         }
//         consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
//         auto node = std::make_unique<CallNode>(
//             SourceLocation(token.line, token.column, token.filename),
//             token.lexeme,
//             std::move(arguments)
//             );
//         currentNode = node.get();
//         return node;
//     }

//     // Otherwise, it's a variable reference
//     auto node = std::make_unique<VariableNode>(
//         SourceLocation(token.line, token.column),
//         Type{TypeTag::Any},
//         token.lexeme,
//         false,
//         std::nullopt
//         );
//     currentNode = node.get();
//     return node;
// }

// std::unique_ptr<ASTNode> PrattParser::parseAssignment(std::unique_ptr<ASTNode> left, Token token) {
//     // Ensure left is a valid assignment target
//     if (dynamic_cast<VariableNode*>(left.get()) == nullptr) {
//         error("Invalid assignment target");
//         return left;
//     }

//     auto right = parseExpression(PREC_ASSIGNMENT);

//     auto node = std::make_unique<AssignmentNode>(
//         SourceLocation(token.line, token.column, token.filename),
//         std::move(left),
//         std::move(right)
//         );

//     currentNode = node.get();
//     return node;
// }

// std::unique_ptr<ASTNode> PrattParser::parseCall(std::unique_ptr<ASTNode> callee, Token token) {
//     std::vector<std::unique_ptr<ASTNode>> arguments;
//     if (!check(TokenType::RIGHT_PAREN)) {
//         do {
//             arguments.push_back(parseExpression(PREC_NONE));
//         } while (match(TokenType::COMMA));
//     }
//     consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");

//     auto* varNode = dynamic_cast<VariableNode*>(callee.get());
//     if (!varNode) {
//         error("Expected variable reference for function call");
//         return callee;
//     }

//     auto node = std::make_unique<CallNode>(
//         SourceLocation(token.line, token.column, token.filename),
//         varNode->getName(),
//         std::move(arguments)
//         );
//     currentNode = node.get();
//     return node;
// }


// // std::unique_ptr<ASTNode> PrattParser::parsePrintStatement(Token token) {
// //     auto expr = parseExpression(PREC_NONE);
// //     consume(TokenType::SEMICOLON, "Expected ';' after print statement");
// //    auto node  = std::make_unique<PrintNode>(token, std::move(expr));
// // }

// std::unique_ptr<ASTNode> PrattParser::parseFnDeclaration(Token token) {
//     consume(TokenType::IDENTIFIER, "Expected function name");
//     std::string name = previous().lexeme;

//     consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
//     std::vector<Parameter> parameters;

//     if (!check(TokenType::RIGHT_PAREN)) {
//         do {
//             consume(TokenType::IDENTIFIER, "Expected parameter name");
//             std::string paramName = previous().lexeme;
//             TypeTag paramType = TypeTag::Any;
//             if (match(TokenType::COLON)) {
//                 Token typeToken = advance();
//                 paramType = stringToType(typeToken.lexeme);
//             }
//             parameters.emplace_back(paramName, paramType);
//         } while (match(TokenType::COMMA));
//     }
//     consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");

//     TypeTag returnType = TypeTag::Nil;
//     if (match(TokenType::COLON)) {
//         Token typeToken = advance();
//         returnType = stringToType(typeToken.lexeme);
//     }

//     consume(TokenType::LEFT_BRACE, "Expected '{' before function body");
//     auto bodyNode = parseBlock(previous());
//     auto bodyStmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(bodyNode.release()));

//     if (!bodyStmt) {
//         error("Function body must be a block statement");
//         return createEmptyNode();
//     }

//     // Convert Statement to vector of Statements
//     std::vector<std::unique_ptr<Statement>> bodyStatements;
//     bodyStatements.push_back(std::move(bodyStmt));

//     auto fnNode = std::make_unique<FunctionNode>(
//         SourceLocation(token.line, token.column, token.filename),
//         name,
//         Type{returnType},
//         parameters,
//         std::move(bodyStatements)
//         );

//     currentNode = fnNode.get();
//     return fnNode;
// }

// std::unique_ptr<ASTNode> PrattParser::parseClassDeclaration(Token token) {
//     // Class name
//     consume(TokenType::IDENTIFIER, "Expected class name");
//     std::string className = previous().lexeme;

//     // Optional parent class
//     std::optional<std::string> parentClass = std::nullopt;
//     if (match(TokenType::COLON)) {
//         consume(TokenType::IDENTIFIER, "Expected parent class name");
//         parentClass = previous().lexeme;
//     }

//     // Class body
//     consume(TokenType::LEFT_BRACE, "Expected '{' before class body");

//     std::vector<std::unique_ptr<Statement>> members;

//     while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
//         if (match(TokenType::FN)) {
//             auto method = parseFnDeclaration(previous());
//             if (auto* methodPtr = dynamic_cast<Statement*>(method.get())) {
//                 // Properly release and move the pointer
//                 method.release();
//                 members.push_back(std::unique_ptr<Statement>(methodPtr));
//             }
//         } else if (match(TokenType::VAR)) {
//             auto field = parseDecVariable(previous());
//             if (auto* fieldPtr = dynamic_cast<Statement*>(field.get())) {
//                 // Properly release and move the pointer
//                 field.release();
//                 members.push_back(std::unique_ptr<Statement>(fieldPtr));
//             }
//         } else {
//             error("Expected method or field declaration in class body");
//             advance();
//         }
//     }

//     consume(TokenType::RIGHT_BRACE, "Expected '}' after class body");

//     auto node = std::make_unique<ClassNode>(
//         SourceLocation(token.line, token.column),
//         className,
//         parentClass,
//         std::move(members)
//         );
//     currentNode = node.get();
//     return node;
// }

// // Create empty AST node
// std::unique_ptr<ASTNode> PrattParser::createEmptyNode() {
//     auto node = std::make_unique<NilNode>(SourceLocation(previous().line, previous().column));
//     currentNode = node.get();
//     return node;
// }

// // Parse return statements
// std::unique_ptr<ASTNode> PrattParser::parseReturnStatement() {
//     Token keyword = previous();
//     std::unique_ptr<ASTNode> value = nullptr;

//     if (!check(TokenType::SEMICOLON)) {
//         value = parseExpression();
//     }

//     consume(TokenType::SEMICOLON, "Expect ';' after return value.");

//     auto node = std::make_unique<ReturnNode>(std::move(value));
//     currentNode = node.get();
//     return node;
// }

// // Parse concurrent statements
// std::unique_ptr<ASTNode> PrattParser::parseConcurrentStatement() {
//     consume(TokenType::LEFT_PAREN, "Expect '(' after 'concurrent'.");
//     auto expression = parseExpression();
//     consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");

//     auto body = parseStatement();

//     // Safely cast and transfer ownership
//     auto expr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(expression.release()));
//     auto stmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(body.release()));

//     if (!expr || !stmt) {
//         error("Invalid operands for concurrent operation");
//         return createEmptyNode();
//     }

//     auto node = std::make_unique<ConcurrentNode>(std::move(expr), std::move(stmt));
//     currentNode = node.get();
//     return node;
// }

// // Parse parallel statements
// std::unique_ptr<ASTNode> PrattParser::parseParallelStatement() {
//     consume(TokenType::LEFT_PAREN, "Expect '(' after 'parallel'.");
//     auto expression = parseExpression();
//     consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");

//     auto body = parseStatement();

//     // Safely cast and transfer ownership
//     auto expr = std::unique_ptr<Expression>(dynamic_cast<Expression*>(expression.release()));
//     auto stmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(body.release()));

//     if (!expr || !stmt) {
//         error("Invalid operands for parallel operation");
//         return createEmptyNode();
//     }

//     auto node = std::make_unique<ParallelNode>(std::move(expr), std::move(stmt));
//     currentNode = node.get();
//     return node;
// }

// // Parse import statements
// std::unique_ptr<ASTNode> PrattParser::parseImport() {
//     consume(TokenType::IDENTIFIER, "Expect module name after 'import'.");
//     std::string moduleName = previous().lexeme;

//     consume(TokenType::SEMICOLON, "Expect ';' after import statement.");

//     auto node = std::make_unique<ImportNode>(
//         SourceLocation(previous().line, previous().column),
//         moduleName
//         );
//     currentNode = node.get();
//     return node;
// }

// // Parse module declarations
// std::unique_ptr<ASTNode> PrattParser::parseModules() {
//     consume(TokenType::IDENTIFIER, "Expect module name after 'module'.");
//     std::string moduleName = previous().lexeme;

//     consume(TokenType::LEFT_BRACE, "Expect '{' after module name.");
//     auto body = parseBlock();

//     auto bodyStmt = std::unique_ptr<Statement>(dynamic_cast<Statement*>(body.release()));
//     if (!bodyStmt) {
//         error("Invalid module body");
//         return createEmptyNode();
//     }

//     auto node = std::make_unique<ModuleNode>(moduleName, std::move(bodyStmt));
//     currentNode = node.get();
//     return node;
// }

// // Parse type declarations
// void PrattParser::parseTypes() {
//     consume(TokenType::IDENTIFIER, "Expect type name.");
//     std::string typeName = previous().lexeme;

//     // Parse type definition
//     if (match(TokenType::EQUAL)) {
//         // Parse type definition here
//         // For enums, structs, etc.
//     }

//     consume(TokenType::SEMICOLON, "Expect ';' after type declaration.");
// }

// // Handle unexpected tokens
// std::unique_ptr<ASTNode> PrattParser::parseUnexpected() {
//     error("Unexpected token: " + previous().lexeme);
//     advance(); // Skip the token
//     auto node = std::make_unique<NilNode>(SourceLocation(previous().line, previous().column));
//     currentNode = node.get();
//     return node;
// }

// // Parse parenthesized expressions
// std::unique_ptr<ASTNode> PrattParser::parseParenthesis() {
//     auto expr = parseExpression();
//     consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
//     auto node = std::make_unique<GroupingNode>(std::move(expr));
//     currentNode = node.get();
//     return node;
// }

// // Check if token is the start of an expression
// bool PrattParser::isExpression(TokenType type) {
//     switch(type) {
//     case TokenType::IDENTIFIER:
//     case TokenType::STRING:
//     case TokenType::NUMBER:
//     case TokenType::TRUE:
//     case TokenType::FALSE:
//     case TokenType::NIL_TYPE:
//     case TokenType::LEFT_PAREN:
//     case TokenType::MINUS:
//     case TokenType::BANG:
//         return true;
//     default:
//         return false;
//     }
// }

// // Load a variable from the environment
// void PrattParser::parseLoadVariable() {
//     Token name = previous();
//     int32_t memoryLocation = getVariableMemoryLocation(name);

//     emit(Opcode::LOAD_VARIABLE, name.line,
//          Value{std::make_shared<Type>(TypeTag::Int), memoryLocation});
// }

// // Parse function calls
// std::unique_ptr<ASTNode> PrattParser::parseFnCall() {
//     Token funcName = previous();

//     consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

//     std::vector<std::unique_ptr<ASTNode>> arguments;
//     if (!check(TokenType::RIGHT_PAREN)) {
//         do {
//             arguments.emplace_back(parseExpression());
//         } while (match(TokenType::COMMA));
//     }

//     consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

//     auto node = std::make_unique<CallNode>(
//         SourceLocation(funcName.line, funcName.column),
//         funcName.lexeme,
//         std::move(arguments)
//         );
//     currentNode = node.get();
//     return node;
// }

// Parse EOF token
std::unique_ptr<ASTNode> PrattParser::parseEOF() {
    return createEmptyNode();
}

// Convert string to type tag
TypeTag PrattParser::stringToType(const std::string &typeStr) {
    auto it = std::find_if(typeMappings.begin(),
                           typeMappings.end(),
                           [&typeStr](const TypeMapping &mapping) {
                               return typeStr == mapping.str;
                           });

    if (it != typeMappings.end()) {
        return it->tag;
    }

    return TypeTag::UserDefined;
}

// Declare a variable
void PrattParser::declareVariable(const Token &name,
                                  const TypePtr &type,
                                  std::optional<ValuePtr> defaultValue) {
    try {
        int32_t memoryLocation = variable.addVariable(name.lexeme, type, false, defaultValue);
        emit(Opcode::DECLARE_VARIABLE,
             name.line,
             Value{std::make_shared<Type>(TypeTag::Int), memoryLocation});
    } catch (const std::runtime_error &e) {
        error(e.what());
    }
}

// Get memory location for a variable
int32_t PrattParser::getVariableMemoryLocation(const Token &name) {
    try {
        return variable.getVariableMemoryLocation(name.lexeme);
    } catch (const std::runtime_error &e) {
        error(e.what());
        return -1;
    }
}
