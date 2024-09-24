#include "packrat.hh"
#include "../debugger.hh"
#include <iostream>
#include <regex>
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
    try {
        auto start_time = std::chrono::high_resolution_clock::now();
        scanner.current = 0;
        program();
        if (pos >= tokens.size()) {
            error("Unexpected input at position " + std::to_string(pos + 1));
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "Parsing completed in " << duration.count() << " microseconds." << std::endl;
        //std::cout << "Parsing debug " << toString() << std::endl;
        return bytecode;
    } catch (const std::exception &e) {
        std::cerr << "Parsing error: " << e.what() << std::endl;
        throw;
    }
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
    } else if ((peek().type == TokenType::IDENTIFIER)
               && (peekNext().type == TokenType::EQUAL || peekNext().type == TokenType::PLUS_EQUAL
                   || peekNext().type == TokenType::MINUS_EQUAL)) {
        assignment();
    } else if (match(TokenType::FN)) {
        function_declaration();
    } else if (match(TokenType::RETURN)) {
        // return_statement();
        if (!check(TokenType::SEMICOLON)) {
            expression();
        }
        consume(TokenType::SEMICOLON, "Expected ';' after return statement.");
        emit(Opcode::RETURN, peek().line);
    } else if (match(TokenType::CLASS)) {
        class_declaration();
    } else {
        expression_statement();
    }
}

void PackratParser::if_statement()
{
    expression(); // condition
    size_t jumpIfFalsePos = bytecode.size();
    emit(Opcode::JUMP_IF_FALSE,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int), 0}); // Placeholder jump

    consume(TokenType::LEFT_BRACE, "Expected '{' after if condition.");
    block();

    size_t jumpPos = bytecode.size();
    emit(Opcode::JUMP,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int), 0}); // Placeholder jump

    size_t elseStart = bytecode.size();
    // Update the JUMP_IF_FALSE instruction with the correct jump location
    bytecode[jumpIfFalsePos].value = std::make_shared<Value>(
        Value{std::make_shared<Type>(TypeTag::Int), elseStart});

    std::vector<size_t> elifJumps;

    while (match(TokenType::ELIF)) {
        expression(); // condition
        size_t elifJumpIfFalsePos = bytecode.size();
        emit(Opcode::JUMP_IF_FALSE,
             peek().line,
             Value{std::make_shared<Type>(TypeTag::Int), 0}); // Placeholder jump

        consume(TokenType::LEFT_BRACE, "Expected '{' after elif condition.");
        block();

        elifJumps.push_back(bytecode.size());
        emit(Opcode::JUMP,
             peek().line,
             Value{std::make_shared<Type>(TypeTag::Int), 0}); // Placeholder jump

        size_t elifEnd = bytecode.size();
        // Update the JUMP_IF_FALSE instruction with the correct jump location
        bytecode[elifJumpIfFalsePos].value = std::make_shared<Value>(
            Value{std::make_shared<Type>(TypeTag::Int), elifEnd});
    }

    if (match(TokenType::ELSE)) {
        consume(TokenType::LEFT_BRACE, "Expected '{' after else.");
        block();
    }

    size_t endIfStatement = bytecode.size();

    // Update all JUMP instructions to the end of the if statement
    bytecode[jumpPos].value = std::make_shared<Value>(
        Value{std::make_shared<Type>(TypeTag::Int), endIfStatement});
    for (size_t elifJump : elifJumps) {
        bytecode[elifJump].value = std::make_shared<Value>(
            Value{std::make_shared<Type>(TypeTag::Int), endIfStatement});
    }
}

void PackratParser::while_statement()
{
    size_t loopStart = bytecode.size();
    expression(); // condition
    size_t jumpIfFalsePos = bytecode.size();
    emit(Opcode::JUMP_IF_FALSE,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int), 100}); // Placeholder jump

    consume(TokenType::LEFT_BRACE, "Expected '{' after while condition.");
    block();
    //fixed the issue with whileloops not working
    int32_t backJump = loopStart - bytecode.size() - 1;
    emit(Opcode::JUMP, peek().line, Value{std::make_shared<Type>(TypeTag::Int), backJump});
    size_t loopEnd = bytecode.size();

    int32_t forwardJump = loopEnd;
    //    int32_t forwardJump = static_cast<int32_t>(loopEnd - jumpIfFalsePos - 1);
    // Update the JUMP_IF_FALSE instruction with the correct jump location
    bytecode[jumpIfFalsePos].value = std::make_shared<Value>(
        Value{std::make_shared<Type>(TypeTag::Int), forwardJump});
}

void PackratParser::for_statement()
{
        consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'.");

    // Initialization
    if (!match(TokenType::SEMICOLON)) {
        var_declaration();
    } else {
        emit(Opcode::NOP, peek().line);
    }

    size_t loopStart = bytecode.size();

    // Condition
    size_t exitJump = 0;
    if (!match(TokenType::SEMICOLON)) {
        expression();
        consume(TokenType::SEMICOLON, "Expected ';' after loop condition.");
        exitJump = bytecode.size();
        emit(Opcode::JUMP_IF_FALSE, peek().line, Value{std::make_shared<Type>(TypeTag::Int), 0}); // Placeholder jump
    }

    // Increment
    size_t bodyJump = bytecode.size();
    emit(Opcode::JUMP, peek().line, Value{std::make_shared<Type>(TypeTag::Int), 0}); // Placeholder jump
    size_t incrementStart = bytecode.size();
    if (!match(TokenType::RIGHT_PAREN)) {
        expression();
        emit(Opcode::JUMP, peek().line, Value{std::make_shared<Type>(TypeTag::Int), loopStart});
        consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses.");
    }

    // Body
    size_t bodyStart = bytecode.size();
    bytecode[bodyJump].value = std::make_shared<Value>(Value{std::make_shared<Type>(TypeTag::Int), bodyStart});
    block();
    emit(Opcode::JUMP, peek().line, Value{std::make_shared<Type>(TypeTag::Int), incrementStart});

    // Update jumps
    size_t loopEnd = bytecode.size();
    if (exitJump != 0) {
        bytecode[exitJump].value = std::make_shared<Value>(Value{std::make_shared<Type>(TypeTag::Int), loopEnd});
    }
}

void PackratParser::print_statement()
{
    consume(TokenType::LEFT_PAREN, "Expected '(' before print expression.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after print expression.");
    consume(TokenType::SEMICOLON, "Expected ';' after the print function.");
    emit(Opcode::PRINT, peek().line);
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
    //    auto start = std::chrono::high_resolution_clock::now();
    //    std::cout << "Starting variable declaration" << std::endl;
    Token name = peek();
    consume(TokenType::IDENTIFIER, "Expected variable name.");

    TypePtr type = std::make_shared<Type>(TypeTag::Int);
    if (match(TokenType::COLON)) {
        //        std::cout << "Variable initialization found for " << name.lexeme << std::endl;
        Token typeToken = peek();
        advance(); //This should check against all the types
        //consume(TokenType::IDENTIFIER, "Expected type name.");
        type = std::make_shared<Type>(stringToType(typeToken.lexeme));
    }

    //    std::cout << "declaration of variables initiated" << std::endl;
    declareVariable(name, type);

    if (match(TokenType::EQUAL)) {
        expression();
        emit(Opcode::STORE_VARIABLE,
             peek().line,
             Value{std::make_shared<Type>(TypeTag::Int), getVariableMemoryLocation(name)});
    } else {
        emit(Opcode::NOP, peek().line);
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");

    //    auto end = std::chrono::high_resolution_clock::now();
    //    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    //    std::cout << "Time taken by <var_declaration>: " << duration << " microseconds\n";
}

void PackratParser::var_call(const Token &name)
{
    int32_t location = getVariableMemoryLocation(name);
    emit(Opcode::LOAD_VARIABLE, peek().line, Value{std::make_shared<Type>(TypeTag::Int), location});
}

void PackratParser::assignment()
{
    auto start = std::chrono::high_resolution_clock::now();
    Token name = peek();
    std::cout << "Starting assignment" << std::endl;
    consume(TokenType::IDENTIFIER, "Expected variable name.");

    TokenType assignmentType = TokenType::EQUAL;
    if (match(TokenType::PLUS_EQUAL)) {
        assignmentType = TokenType::PLUS_EQUAL;
    } else if (match(TokenType::MINUS_EQUAL)) {
        assignmentType = TokenType::MINUS_EQUAL;
    } else {
        consume(TokenType::EQUAL, "Expected '=', '+=', or '-=' after variable name.");
    }

    std::cout << "Variable " << name.lexeme << " assigned" << std::endl;
    expression();
    consume(TokenType::SEMICOLON, "Expected ';' after assignment.");

    int32_t location = getVariableMemoryLocation(name);

    if (assignmentType == TokenType::PLUS_EQUAL) {
        emit(Opcode::LOAD_VARIABLE,
             peek().line,
             Value{std::make_shared<Type>(TypeTag::Int), location});
        emit(Opcode::ADD, peek().line);
    } else if (assignmentType == TokenType::MINUS_EQUAL) {
        emit(Opcode::LOAD_VARIABLE,
             peek().line,
             Value{std::make_shared<Type>(TypeTag::Int), location});
        emit(Opcode::SUBTRACT, peek().line);
    }

    emit(Opcode::STORE_VARIABLE, peek().line, Value{std::make_shared<Type>(TypeTag::Int), location});

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time taken by <assignment>: " << duration << " microseconds\n";
}

void PackratParser::function_declaration()
{
    Token name = peek();
    consume(TokenType::IDENTIFIER, "Expected function name.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name.");

    std::vector<std::pair<std::string, TypePtr>> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token paramName = peek();
            consume(TokenType::IDENTIFIER, "Expected parameter name.");
            TypePtr paramType = nullptr;
            if (match(TokenType::COLON)) {
                Token typeToken = peek();
                advance();
                paramType = std::make_shared<Type>(stringToType(typeToken.lexeme));
            }
            parameters.push_back({paramName.lexeme, paramType});
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters.");

    TypePtr returnType = nullptr;
    if (match(TokenType::COLON)) {
        Token typeToken = peek();
        advance(); //This should check against all the types
        //consume(TokenType::IDENTIFIER, "Expected return type name.");
        returnType = std::make_shared<Type>(stringToType(typeToken.lexeme));
    }

    consume(TokenType::LEFT_BRACE, "Expected '{' before function body.");

    enterScope();

    // Emit function definition
    emit(Opcode::DEFINE_FUNCTION,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::Int), name.lexeme});

    // Add parameters to the current scope
    for (const auto &param : parameters) {
        declareVariable(Token{TokenType::IDENTIFIER, param.first}, param.second);
    }

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statement();
    }
    // Emit return if not present
    if (bytecode.back().opcode != Opcode::RETURN) {
        if (returnType && returnType->tag != TypeTag::Nil) {
            error("Function must return a value of type " + returnType->toString());
        }
        emit(Opcode::RETURN, peek().line);
    }
    consume(TokenType::RIGHT_BRACE, "Expected '}' after function block.");

    exitScope();
}

void PackratParser::function_call(const Token &name)
{
    std::vector<TypePtr> argTypes;
    int argCount = 0;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            expression();
            argCount++;
            // You might want to infer and store argument types here
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments.");

    emit(Opcode::INVOKE_FUNCTION,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::String), name.lexeme});
    emit(Opcode::PUSH_ARGS, peek().line, Value{std::make_shared<Type>(TypeTag::Int), argCount});
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
    Token token = peek();
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
        parse_string();
    } else if (match(TokenType::IDENTIFIER)) {
        handle_identifier();
    } else if (match(TokenType::LEFT_PAREN)) {
        expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
    } else {
        error("Expected expression.");
    }
}
void PackratParser::parse_string()
{
    Token stringToken = previous();
    interpolate_string(stringToken.lexeme);
}

void PackratParser::interpolate_string(const std::string &str)
{
    std::regex interpolation_regex("\\{([^}]+)\\}");
    std::string::const_iterator searchStart(str.cbegin());
    std::smatch match;

    //    // Load the initial string part
    //    emit(Opcode::LOAD_STR, peek().line, Value{std::make_shared<Type>(TypeTag::String), str});
    std::string interpolatedString = std::regex_replace(str, interpolation_regex, "{}");
    emit(Opcode::LOAD_STR,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::String), interpolatedString});

    while (std::regex_search(searchStart, str.cend(), match, interpolation_regex)) {
        std::string expr = match[1].str();

        // Handle variable interpolation
        if (variable.hasVariable(expr)) {
            // Emit code to load the variable
            int32_t memoryLocation = variable.getVariableMemoryLocation(expr);
            emit(Opcode::LOAD_VARIABLE,
                 peek().line,
                 Value{std::make_shared<Type>(TypeTag::Int), memoryLocation});
        } else {
            // If it's not a variable, treat it as an expression
            std::vector<Token> exprTokens = tokenizeExpression(expr);

            // Save current parser state
            size_t savedPos = pos;
            std::vector<Token> savedTokens = tokens;

            // Set up parser state for the interpolated expression
            tokens = exprTokens;
            pos = 0;

            // Parse and evaluate the expression
            expression();

            // Restore parser state
            pos = savedPos;
            tokens = savedTokens;
        }

        // Emit the INTERPOLATE_STRING instruction
        emit(Opcode::INTERPOLATE_STRING, peek().line);

        searchStart = match.suffix().first;
    }
}

std::vector<Token> PackratParser::tokenizeExpression(const std::string &expr)
{
    // Create a new Scanner object
    Scanner exprScanner(expr, "interpolation", "");

    // Scan the expression
    std::vector<Token> exprTokens = exprScanner.scanTokens();

    // Remove the EOF token if present
    if (!exprTokens.empty() && exprTokens.back().type == TokenType::EOF_TOKEN) {
        exprTokens.pop_back();
    }

    return exprTokens;
}

void PackratParser::handle_identifier()
{
    Token name = previous();
    if (match(TokenType::LEFT_PAREN)) {
        // Function call
        function_call(name);
    } else if (match(TokenType::DOT)) {
        // Method or class call
        method_call(name);
    } else {
        // Variable call
        var_call(name);
    }
}

void PackratParser::method_call(const Token &object)
{
    Token method = peek();
    consume(TokenType::IDENTIFIER, "Expected method name after '.'.");

    if (match(TokenType::LEFT_PAREN)) {
        int argCount = 0;
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                expression();
                argCount++;
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments.");

        emit(Opcode::METHOD_CALL,
             peek().line,
             Value{std::make_shared<Type>(TypeTag::String), object.lexeme + "." + method.lexeme});
        emit(Opcode::PUSH_ARGS, peek().line, Value{std::make_shared<Type>(TypeTag::Int), argCount});
    } else {
        // This is a property access, not a method call
        emit(Opcode::LOAD_PROPERTY,
             peek().line,
             Value{std::make_shared<Type>(TypeTag::String), object.lexeme + "." + method.lexeme});
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
    //    auto start = std::chrono::high_resolution_clock::now();
    //    std::cout << "Declaring variable " << name.lexeme << std::endl;
    int32_t memoryLocation = variable.addVariable(name.lexeme, type, false, defaultValue);
    emit(Opcode::DECLARE_VARIABLE,
         name.line,
         Value{std::make_shared<Type>(TypeTag::Int), memoryLocation});
    //    auto end = std::chrono::high_resolution_clock::now();
    //    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    //    std::cout << "Time taken by <declareVar>: " << duration << " microseconds\n";
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
    case TypeTag::Any:
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
        error("Sum and UserDefined types are not supported in this setValue function");
        // These types might require more complex parsing logic
    default:
        error("Unsupported type for value setting: " + type->toString());
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
    case TokenType::STR_TYPE:
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
    if (pos > 0) {
        return tokens[pos - 1];
    }
    return tokens[0]; // Return the first token if there is no previous one
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
        error(message);
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
    bool result = peek().type == TokenType::EOF_TOKEN;
    if (result) {
        emit(Opcode::HALT, peek().line);
    }
    return result;
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
