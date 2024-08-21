#include "packrat.hh"
#include "../debugger.hh"
#include <iostream>
#include <sstream>

PackratParser::PackratParser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem)
    : scanner(scanner)
    , variable(typeSystem)
    , typeSystem(typeSystem)
{
    tokens = scanner.scanTokens();
    pos = 0;
}

Bytecode PackratParser::parse()
{
    program();
    if (pos < tokens.size()) {
        throw std::runtime_error("Unexpected input at position " + std::to_string(pos));
    }
    return bytecode;
}

void PackratParser::program()
{
    while (!isAtEnd()) {
        statement();
    }
}

void PackratParser::statement()
{
    if (match(TokenType::IF)) {
        if_statement();
    } else if (match(TokenType::WHILE)) {
        while_statement();
    } else if (match(TokenType::FOR)) {
        for_statement();
    } else if (match(TokenType::PRINT)) {
        print_statement();
    } else if (match(TokenType::LEFT_BRACE)) {
        block();
    } else if (match(TokenType::VAR)) {
        var_declaration();
    } else if (peek().type == TokenType::IDENTIFIER && peekNext().type == TokenType::EQUAL) {
        assignment();
    } else if (match(TokenType::FN)) {
        function_declaration();
    } else if (match(TokenType::CLASS)) {
        class_declaration();
    } else {
        expression_statement();
    }
}

void PackratParser::if_statement()
{
    expression(); // condition
    emit(Opcode::JUMP_IF_FALSE, peek().line);
    size_t jumpIfFalsePos = bytecode.size() - 1;

    consume(TokenType::LEFT_BRACE, "Expected '{' after if condition.");
    block();

    emit(Opcode::JUMP, peek().line);
    size_t jumpPos = bytecode.size() - 1;
    bytecode[jumpIfFalsePos].value = std::make_shared<Value>(
        Value{std::make_shared<Type>(TypeTag::Int32), bytecode.size()});

    if (match(TokenType::ELIF)) {
        do {
            expression(); // condition
            emit(Opcode::JUMP_IF_FALSE, peek().line);
            size_t jumpIfFalsePos = bytecode.size() - 1;

            consume(TokenType::LEFT_BRACE, "Expected '{' after elif condition.");
            block();

            emit(Opcode::JUMP, peek().line);
            size_t jumpPos = bytecode.size() - 1;
            bytecode[jumpIfFalsePos].value = std::make_shared<Value>(
                Value{std::make_shared<Type>(TypeTag::Int32), bytecode.size()});
        } while (match(TokenType::ELIF));

        if (match(TokenType::ELSE)) {
            consume(TokenType::LEFT_BRACE, "Expected '{' after else.");
            block();
        }
    } else if (match(TokenType::ELSE)) {
        consume(TokenType::LEFT_BRACE, "Expected '{' after else.");
        block();
    }

    bytecode[jumpPos].value = std::make_shared<Value>(
        Value{std::make_shared<Type>(TypeTag::Int32), bytecode.size()});
}

void PackratParser::while_statement()
{
    size_t loopStart = bytecode.size();
    expression(); // condition
    emit(Opcode::JUMP_IF_FALSE, peek().line);
    size_t jumpIfFalsePos = bytecode.size() - 1;

    consume(TokenType::LEFT_BRACE, "Expected '{' after while condition.");
    block();

    emit(Opcode::JUMP, peek().line, Value{std::make_shared<Type>(TypeTag::Int), loopStart});
    bytecode[jumpIfFalsePos].value = std::make_shared<Value>(
        Value{std::make_shared<Type>(TypeTag::Int32), bytecode.size()});
}

void PackratParser::for_statement()
{
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'.");

    if (!match(TokenType::SEMICOLON)) {
        var_declaration();
    } else {
        emit(Opcode::NOP, peek().line);
    }

    size_t loopStart = bytecode.size();
    size_t exitJump = 0;
    if (!match(TokenType::SEMICOLON)) {
        expression();
        consume(TokenType::SEMICOLON, "Expected ';' after loop condition.");
        emit(Opcode::JUMP_IF_FALSE, peek().line);
        exitJump = bytecode.size() - 1;
    }

    if (!match(TokenType::RIGHT_PAREN)) {
        size_t bodyJump = bytecode.size();
        emit(Opcode::JUMP, peek().line);
        size_t incrementStart = bytecode.size();
        expression();
        //emit(Opcode::POP, peek().line);
        emit(Opcode::JUMP, peek().line, Value{std::make_shared<Type>(TypeTag::Int), loopStart});
        bytecode[bodyJump].value = std::make_shared<Value>(
            Value{std::make_shared<Type>(TypeTag::Int32), bytecode.size()});
        consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses.");
    }

    block();
    emit(Opcode::JUMP, peek().line, Value{std::make_shared<Type>(TypeTag::Int), loopStart});

    if (exitJump != 0) {
        bytecode[exitJump].value = std::make_shared<Value>(
            Value{std::make_shared<Type>(TypeTag::Int32), bytecode.size()});
    }
}

void PackratParser::print_statement()
{
    expression();
    emit(Opcode::PRINT, peek().line);
    consume(TokenType::SEMICOLON, "Expected ';' after value.");
}

void PackratParser::block()
{
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statement();
    }

    consume(TokenType::RIGHT_BRACE, "Expected '}' after block.");
}

void PackratParser::var_declaration()
{
    Token name = peek();
    consume(TokenType::IDENTIFIER, "Expected variable name.");

    TypePtr type = nullptr;
    if (match(TokenType::COLON)) {
        Token typeToken = peek();
        consume(TokenType::IDENTIFIER, "Expected type name.");
        type = std::make_shared<Type>(stringToType(typeToken.lexeme));
    }

    if (match(TokenType::EQUAL)) {
        expression();
    } else {
        emit(Opcode::NOP, peek().line);
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");

    declareVariable(name, type);
}

void PackratParser::assignment()
{
    Token name = peek();
    consume(TokenType::IDENTIFIER, "Expected variable name.");
    consume(TokenType::EQUAL, "Expected '=' after variable name.");
    expression();
    consume(TokenType::SEMICOLON, "Expected ';' after assignment.");

    int32_t location = getVariableMemoryLocation(name);
    emit(Opcode::STORE_VARIABLE, peek().line, Value{std::make_shared<Type>(TypeTag::Int), location});
}

void PackratParser::function_declaration()
{
    Token name = peek();
    consume(TokenType::IDENTIFIER, "Expected function name.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name.");

    std::vector<TypePtr> paramTypes;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token paramName = peek();
            consume(TokenType::IDENTIFIER, "Expected parameter name.");
            TypePtr paramType = nullptr;
            if (match(TokenType::COLON)) {
                Token typeToken = peek();
                consume(TokenType::IDENTIFIER, "Expected type name.");
                paramType = std::make_shared<Type>(stringToType(typeToken.lexeme));
            }
            paramTypes.push_back(paramType);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters.");

    TypePtr returnType = nullptr;
    if (match(TokenType::COLON)) {
        Token typeToken = peek();
        consume(TokenType::IDENTIFIER, "Expected return type name.");
        returnType = std::make_shared<Type>(stringToType(typeToken.lexeme));
    }

    consume(TokenType::LEFT_BRACE, "Expected '{' before function body.");

    enterScope();

    // Emit function definition
    emit(Opcode::DEFINE_FUNCTION,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int), name.lexeme});

    block();

    exitScope();

    // Emit return if not present
    if (bytecode.back().opcode != Opcode::RETURN) {
        //emit(Opcode::NIL, peek().line);
        emit(Opcode::RETURN, peek().line);
    }
}

void PackratParser::class_declaration()
{
    Token name = peek();
    consume(TokenType::IDENTIFIER, "Expected class name.");
    consume(TokenType::LEFT_BRACE, "Expected '{' before class body.");

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        function_declaration();
    }

    consume(TokenType::RIGHT_BRACE, "Expected '}' after class body.");

    // Emit class definition
    emit(Opcode::DEFINE_CLASS,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::String), name.lexeme});
}

void PackratParser::expression_statement()
{
    expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    // emit(Opcode::POP, peek().line);
}

void PackratParser::expression()
{
    logical_or_expression();
}

void PackratParser::logical_or_expression()
{
    logical_and_expression();

    while (match(TokenType::OR)) {
        logical_and_expression();
        emit(Opcode::OR, peek().line);
    }
}

void PackratParser::logical_and_expression()
{
    equality_expression();

    while (match(TokenType::AND)) {
        equality_expression();
        emit(Opcode::AND, peek().line);
    }
}

void PackratParser::equality_expression()
{
    comparison_expression();

    while (match(TokenType::EQUAL_EQUAL) || match(TokenType::BANG_EQUAL)) {
        TokenType operatorType = previous().type;
        comparison_expression();

        if (operatorType == TokenType::EQUAL_EQUAL) {
            emit(Opcode::EQUAL, peek().line);
        } else {
            emit(Opcode::NOT_EQUAL, peek().line);
        }
    }
}

void PackratParser::comparison_expression()
{
    additive_expression();

    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) || match(TokenType::LESS)
           || match(TokenType::LESS_EQUAL)) {
        TokenType operatorType = previous().type;
        additive_expression();

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
        default:
            break; // Unreachable
        }
    }
}

void PackratParser::additive_expression()
{
    multiplicative_expression();

    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        TokenType operatorType = previous().type;
        multiplicative_expression();

        if (operatorType == TokenType::PLUS) {
            emit(Opcode::ADD, peek().line);
        } else {
            emit(Opcode::SUBTRACT, peek().line);
        }
    }
}

void PackratParser::multiplicative_expression()
{
    unary_expression();

    while (match(TokenType::STAR) || match(TokenType::SLASH)) {
        TokenType operatorType = previous().type;
        unary_expression();

        if (operatorType == TokenType::STAR) {
            emit(Opcode::MULTIPLY, peek().line);
        } else {
            emit(Opcode::DIVIDE, peek().line);
        }
    }
}

void PackratParser::unary_expression()
{
    if (match(TokenType::BANG) || match(TokenType::MINUS)) {
        TokenType operatorType = previous().type;
        unary_expression();

        if (operatorType == TokenType::BANG) {
            emit(Opcode::NOT, peek().line);
        } else {
            emit(Opcode::NEGATE, peek().line);
        }
    } else {
        primary_expression();
    }
}

void PackratParser::primary_expression()
{
    Token token = previous();
    TypePtr typePtr = std::make_shared<Type>(inferType(token));
    Value value = setValue(typePtr, token.lexeme);
    if (match(TokenType::FALSE)) {
        emit(Opcode::BOOLEAN, peek().line, Value{std::make_shared<Type>(TypeTag::Bool), false});
    } else if (match(TokenType::TRUE)) {
        emit(Opcode::BOOLEAN, peek().line, Value{std::make_shared<Type>(TypeTag::Bool), true});
    } else if (match(TokenType::NIL_TYPE)) {
        emit(Opcode::NOP, peek().line);
    } else if (match(TokenType::NUMBER)) {
        emit(Opcode::LOAD_CONST, peek().line, std::move(value));
    } else if (match(TokenType::STRING)) {
        emit(Opcode::LOAD_STR, peek().line, std::move(value));
    } else if (match(TokenType::IDENTIFIER)) {
        int32_t location = getVariableMemoryLocation(previous());
        emit(Opcode::LOAD_VARIABLE,
             peek().line,
             Value{std::make_shared<Type>(TypeTag::Int), location});
    } else if (match(TokenType::LEFT_PAREN)) {
        expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
    } else {
        throw std::runtime_error("Expected expression.");
    }
}

Instruction PackratParser::emit(Opcode opcode, uint32_t lineNumber)
{
    Instruction instruction(opcode, lineNumber);
    instruction.debug();
    bytecode.push_back(instruction);
    return instruction;
}

Instruction PackratParser::emit(Opcode opcode, uint32_t lineNumber, Value &&value)
{
    ValuePtr valuePtr = std::make_shared<Value>(std::move(value));
    Instruction instruction(opcode, lineNumber, valuePtr);
    instruction.debug();
    bytecode.push_back(instruction);
    return instruction;
}

void PackratParser::declareVariable(const Token &name,
                                    const TypePtr &type,
                                    std::optional<ValuePtr> defaultValue)
{
    int32_t memoryLocation = variable.addVariable(name.lexeme, type, false, defaultValue);
    emit(Opcode::DECLARE_VARIABLE,
         name.line,
         Value{std::make_shared<Type>(TypeTag::Int), memoryLocation});
}

int32_t PackratParser::getVariableMemoryLocation(const Token &name)
{
    return variable.getVariableMemoryLocation(name.lexeme);
}

void PackratParser::enterScope()
{
    variable.enterScope();
}

void PackratParser::exitScope()
{
    variable.exitScope();
}

void PackratParser::error(const std::string &message)
{
    hadError = true;
    Debugger::error(message, peek(), InterpretationStage::PARSING, scanner.getSource());
}

Value PackratParser::setValue(TypePtr type, const std::string &input)
{
    Value value;
    value.type = type;

    switch (type->tag) {
    case TypeTag::Bool:
        value.data = (input == "true");
        break;
    case TypeTag::Int:
    case TypeTag::Int64:
        value.data = static_cast<int64_t>(std::stoll(input));
        break;
    case TypeTag::Int8:
        value.data = static_cast<int8_t>(std::stol(input));
        break;
    case TypeTag::Int16:
        value.data = static_cast<int16_t>(std::stol(input));
        break;
    case TypeTag::Int32:
        value.data = static_cast<int32_t>(std::stol(input));
        break;
    case TypeTag::UInt:
    case TypeTag::UInt64:
        value.data = static_cast<uint64_t>(std::stoull(input));
        break;
    case TypeTag::UInt8:
        value.data = static_cast<uint8_t>(std::stoull(input));
        break;
    case TypeTag::UInt16:
        value.data = static_cast<uint16_t>(std::stoull(input));
        break;
    case TypeTag::UInt32:
        value.data = static_cast<uint32_t>(std::stoull(input));
        break;
    case TypeTag::Float32:
        value.data = std::stof(input);
        break;
    case TypeTag::Float64:
        value.data = std::stod(input);
        break;
    case TypeTag::String:
        value.data = input;
        break;
    case TypeTag::List:
        // Assuming input is a comma-separated list of values
        {
            //                    ListValue listValue;
            //                    std::istringstream iss(input);
            //                    std::string item;
            //                    while (std::getline(iss, item, ',')) {
            //                        listValue.elements.push_back(setValue(type->elementType, item));
            //                    }
            //                    value.data = listValue;
        }
        break;
    case TypeTag::Dict:
        // Assuming input is in the format "key1:value1,key2:value2"
        {
            //                    DictValue dictValue;
            //                    std::istringstream iss(input);
            //                    std::string pair;
            //                    while (std::getline(iss, pair, ',')) {
            //                        size_t colonPos = pair.find(':');
            //                        if (colonPos != std::string::npos) {
            //                            std::string key = pair.substr(0, colonPos);
            //                            std::string val = pair.substr(colonPos + 1);
            //                            dictValue.elements[setValue(type->tag, key)] = setValue(type->tag, val);
            //                        }
            //                    }
            //                    value.data = dictValue;
        }
        break;
    case TypeTag::Sum:
    case TypeTag::UserDefined:
        // These types might require more complex parsing logic
        throw std::runtime_error(
            "Sum and UserDefined types are not supported in this setValue function");
    default:
        throw std::runtime_error("Unsupported type for value setting: " + type->toString());
    }

    return value;
}

TypeTag PackratParser::inferType(const Token &token)
{
    switch (token.type) {
    case TokenType::NUMBER:
        // Check if the number contains a decimal point
        if (token.lexeme.find('.') != std::string::npos) {
            return TypeTag::Float64;
        } else {
            return TypeTag::Int;
        }
    case TokenType::STRING:
        return TypeTag::String;
    case TokenType::TRUE:
    case TokenType::FALSE:
        return TypeTag::Bool;
    case TokenType::NIL_TYPE:
        return TypeTag::Nil; // or create a Null type if needed
    case TokenType::INT_TYPE:
        return TypeTag::Int;
    case TokenType::INT8_TYPE:
        return TypeTag::Int8;
    case TokenType::INT16_TYPE:
        return TypeTag::Int16;
    case TokenType::INT32_TYPE:
        return TypeTag::Int32;
    case TokenType::INT64_TYPE:
        return TypeTag::Int64;
    case TokenType::UINT_TYPE:
        return TypeTag::UInt;
    case TokenType::UINT8_TYPE:
        return TypeTag::UInt8;
    case TokenType::UINT16_TYPE:
        return TypeTag::UInt16;
    case TokenType::UINT32_TYPE:
        return TypeTag::UInt32;
    case TokenType::UINT64_TYPE:
        return TypeTag::UInt64;
    case TokenType::FLOAT32_TYPE:
        return TypeTag::Float32;
    case TokenType::FLOAT_TYPE:
    case TokenType::FLOAT64_TYPE:
        return TypeTag::Float64;
    case TokenType::SUM_TYPE:
        return TypeTag::Sum;
    case TokenType::ANY_TYPE:
        return TypeTag::Any;
    case TokenType::UNION_TYPE:
        return TypeTag::Union;
    case TokenType::USER_TYPE:
        return TypeTag::UserDefined;
    case TokenType::BOOL_TYPE:
        return TypeTag::Bool;
    case TokenType::FUNCTION_TYPE:
        return TypeTag::Function;
    default:
        return TypeTag::Any; // Default to Any for unknown types
    }
}

TypeTag PackratParser::stringToType(const std::string &typeStr)
{
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

Token PackratParser::peek()
{
    return tokens[pos];
}

Token PackratParser::peekNext()
{
    if (pos + 1 < tokens.size()) {
        return tokens[pos + 1];
    }
    return tokens[pos]; // Return current if there's no next token
}

Token PackratParser::previous()
{
    return tokens[pos - 1];
}

void PackratParser::advance()
{
    if (!isAtEnd()) {
        pos++;
    }
}

void PackratParser::consume(TokenType type, const std::string &message)
{
    if (check(type)) {
        advance();
    } else {
        throw std::runtime_error(message);
    }
}

bool PackratParser::match(TokenType type)
{
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool PackratParser::check(TokenType type)
{
    if (isAtEnd()) {
        return false;
    }
    return peek().type == type;
}

bool PackratParser::isAtEnd()
{
    return peek().type == TokenType::EOF_TOKEN;
}

bool PackratParser::isExpression(TokenType type)
{
    switch (type) {
    case TokenType::NUMBER:
    case TokenType::STRING:
    case TokenType::TRUE:
    case TokenType::FALSE:
    case TokenType::NIL_TYPE:
    case TokenType::IDENTIFIER:
    case TokenType::LEFT_PAREN:
    case TokenType::MINUS:
    case TokenType::BANG:
        return true;
    default:
        return false;
    }
}

std::string PackratParser::toString() const
{
    std::stringstream ss;
    ss << "PackratParser state:\n";
    ss << "Current position: " << pos << "\n";
    ss << "Tokens:\n";
    for (size_t i = 0; i < tokens.size(); ++i) {
        ss << (i == pos ? " -> " : "    ") << tokens[i].lexeme << "\n";
    }
    ss << "Bytecode:\n";
    for (const auto &instruction : bytecode) {
        ss << "Instruction: " + instruction.opcodeToString(instruction.opcode)
                  + " | Line: " + std::to_string(instruction.lineNumber) + "\n";
        std::string valueStr;
        std::visit(
            [&valueStr](const auto &val) {
                std::stringstream ss;
                ss << val;
                valueStr = ss.str();
            },
            instruction.value->data);

        ss << " | Value: " + valueStr;
        ss << "\n";
    }
    return ss.str();
}

void PackratParser::parseTypes()
{
    inferType(peek());
}

std::vector<Instruction> PackratParser::getBytecode() const
{
    return bytecode;
}
