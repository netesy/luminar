#include "packrat.hh"
#include "../debugger.hh"
#include "../function.hh"
#include "../optimizer.hh"
#include <iostream>
#include <regex>
#include <sstream>

PackratParser::PackratParser(Scanner &scanner, std::shared_ptr<TypeSystem> typeSystem, Functions &funcs)
    : scanner(scanner)
    , variable(typeSystem)
    , functions(funcs)
    , typeSystem(typeSystem)
{
    tokens = scanner.scanTokens();
    scanner.toString();
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

        // Measure time for bytecode optimization
        auto optimization_start_time = std::chrono::high_resolution_clock::now();
       // bytecode = BytecodeOptimizer::optimize(bytecode);
        auto optimization_end_time = std::chrono::high_resolution_clock::now();
        auto optimization_duration = std::chrono::duration_cast<std::chrono::microseconds>(optimization_end_time - optimization_start_time);

        std::cout << "Bytecode Optimizations completed in " << optimization_duration.count() << " microseconds." << std::endl;
        //std::cout << "Parsing debug " << toString() << std::endl;

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cout << "Parsing completed in " << duration.count() << " microseconds." << std::endl;
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
           // Look ahead to determine if it's a block or a dictionary
        if (check(TokenType::RIGHT_BRACE)) {
            // Empty dictionary
            dict_statement();
        } else if (check(TokenType::STRING) || check(TokenType::IDENTIFIER)) {
            // Check if the next token after STRING or IDENTIFIER is a COLON
            if (peekNext().type == TokenType::COLON) {
                // It's a dictionary
                dict_statement();
            } else {
                // It's a block
                block();
            }
        } else {
            // It's a block
            block();
        }
    } else if (match(TokenType::LEFT_BRACKET)) {
        // Look ahead to determine if it's a list or list index
        if (check(TokenType::RIGHT_BRACKET)) {
            // Empty list
            list_statement();
        } else if (check(TokenType::NUMBER) || check(TokenType::IDENTIFIER)) {
            // Check if the next token after NUMBER or IDENTIFIER is a COMMA or RIGHT_BRACKET
            if (peekNext().type == TokenType::COMMA || peekNext().type == TokenType::RIGHT_BRACKET) {
                // It's a list
                list_statement();
            } else {
                // It's a list index
                //list_index_statement();//!TODO implement this later. 
            }
        } else {
            // It's a list
            list_statement();
        }
    } else if (match(TokenType::VAR)) {
        var_declaration();
    }
    else if ((peek().type == TokenType::IDENTIFIER) && (peekNext().type == TokenType::EQUAL || peekNext().type == TokenType::PLUS_EQUAL || peekNext().type == TokenType::MINUS_EQUAL))
    {
        assignment();
    }
    else if (match(TokenType::FN))
    {
        function_declaration();
    }
    else if (match(TokenType::MATCH))
    {
        match_statement();
    }
    else if (match(TokenType::RETURN))
    {
        // return_statement();
        if (!check(TokenType::SEMICOLON)) {
            expression();
        }
        consume(TokenType::SEMICOLON, "Expected ';' after return statement.");
        emit(Opcode::RETURN, peek().line);
    }
    else if (match(TokenType::RANGE))
    {
        range_function();
    }
    else if (match(TokenType::CLASS))
    {
        class_declaration();
    }
    else
    {
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

void PackratParser::match_statement()
{
    // Parse the `match` expression.
    expression(); // This will evaluate the value to be matched.
    size_t matchValuePos = bytecode.size();

    // Consume the LEFT_BRACE '{' after the match expression.
    consume(TokenType::LEFT_BRACE, "Expected '{' after match expression.");

    std::vector<size_t> caseJumpPositions; // Track positions to update jumps.
    size_t defaultCaseJumpPos = 0;

    // Parse individual cases
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match(TokenType::IDENTIFIER)) {
            Token caseType = previous(); // Get case type identifier (e.g., int, str, list<T>).

            // Check for generics if the type can have them (e.g., list<T> or dict<K, V>).
            // if (match(TokenType::LESS)) { // If '<' is present, it's a generic type.
            //     // Parse the generic type (e.g., T in list<T> or K, V in dict<K, V>).
            //     // This part can be extended for complex generic parsing.
            //     parse_generic_type();
            //     consume(TokenType::GREATER, "Expected '>' after generic type parameters.");
            // }

            // Emit code to check if `value` matches `caseType`.
            size_t caseMatchJump = bytecode.size();
            emit(Opcode::MATCH_TYPE,
                 peek().line,
                 Value{std::make_shared<Type>(TypeTag::String),
                       caseType.lexeme}); // Placeholder for type check.

            // Emit jump if the type doesn't match, to skip to the next case.
            size_t caseSkipJump = bytecode.size();
            emit(Opcode::JUMP_IF_FALSE,
                 peek().line,
                 Value{std::make_shared<Type>(TypeTag::Int), 0}); // Placeholder jump.

            // Expect a block for the case
            consume(TokenType::LEFT_BRACE, "Expected '{' after case pattern.");
            block();

            // Record position for future updates.
            caseJumpPositions.push_back(caseSkipJump);

            // Emit a jump to skip the rest of the match cases once matched.
            size_t endMatchJump = bytecode.size();
            emit(Opcode::JUMP,
                 peek().line,
                 Value{std::make_shared<Type>(TypeTag::Int), 0}); // Placeholder jump.
            caseJumpPositions.push_back(endMatchJump);
        } else if (match(TokenType::DEFAULT)) {
            // Default case `_` - Handle unmatched patterns
            consume(TokenType::LEFT_BRACE, "Expected '{' after default case.");
            block();
            defaultCaseJumpPos = bytecode.size(); // Record position for updating.
        }
    }

    // After parsing all cases, update jumps for case endings.
    size_t endMatch = bytecode.size();
    for (size_t pos : caseJumpPositions) {
        bytecode[pos].value = std::make_shared<Value>(
            Value{std::make_shared<Type>(TypeTag::Int), endMatch});
    }

    // Update default case jump to end of the match statement, if present.
    if (defaultCaseJumpPos != 0) {
        bytecode[defaultCaseJumpPos].value = std::make_shared<Value>(
            Value{std::make_shared<Type>(TypeTag::Int), endMatch});
    }

    // Consume the RIGHT_BRACE '}' to close the match block.
    consume(TokenType::RIGHT_BRACE, "Expected '}' after match cases.");
}

void PackratParser::parse_generic_type()
{
    // Parse generic types, e.g., T in list<T> or K, V in dict<K, V>.
    // This function can be expanded as needed for complex generic handling.
    if (match(TokenType::IDENTIFIER)) {
        // Parse single generic type like T in list<T>
        Token genericType = previous();
        // Additional handling or storage can go here.
    } else if (match(TokenType::IDENTIFIER)) {
        // Parse pairs like K, V in dict<K, V>
        Token keyType = previous();
        consume(TokenType::COMMA, "Expected ',' between generic types.");
        Token valueType = peek();
        consume(TokenType::IDENTIFIER, "Expected value type after key type.");
        // Additional handling or storage can go here.
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

void PackratParser::list_statement()
{
    //  consume(TokenType::LEFT_BRACKET, "Expected '[' to start a list.");

    // Assuming we have a list type prepared
    TypePtr listType = std::make_shared<Type>(TypeTag::List,
                                              ListType{/* no element type specified */});
    ListValue elements;
    while (!check(TokenType::RIGHT_BRACKET) && !isAtEnd()) {
        // Extract lexeme from token as string
        std::string elementStr = peek().lexeme;

        // Directly create a ValuePtr with appropriate type and data
        ValuePtr element = std::make_shared<Value>(
            Value{std::make_shared<Type>(inferType(peek())), // Example type
                  peek().lexeme});

        elements.elements.push_back(element);
        // Parse each list element as an expression
        expression();
        if (!match(TokenType::COMMA))
            break;
    }
    consume(TokenType::RIGHT_BRACKET, "Expected ']' to close the list.");

    emit(Opcode::LOAD_VALUE, peek().line, Value{listType, elements});
}

void PackratParser::dict_statement() {
  //  consume(TokenType::LEFT_BRACE, "Expected '{' to start a dictionary.");
    TypePtr dictType = std::make_shared<Type>(TypeTag::Dict);
    DictValue keyValuePairs;
    
    // Handle empty dictionary case
    if (check(TokenType::RIGHT_BRACE)) {
        advance();
        emit(Opcode::LOAD_VALUE, peek().line, Value{dictType, keyValuePairs});
        return;
    }

    do {
        // Parse key - must be a string or identifier
        if (!check(TokenType::STRING) && !check(TokenType::IDENTIFIER)) {
            error("Dictionary key must be a string or identifier");
            return;
        }

        ValuePtr key = std::make_shared<Value>(
            Value{std::make_shared<Type>(inferType(peek())),
                  peek().lexeme});
        advance();

        consume(TokenType::COLON, "Expected ':' after dictionary key.");

        // Parse value directly without calling expression()
        ValuePtr value = std::make_shared<Value>(
            Value{std::make_shared<Type>(inferType(peek())),
                  peek().lexeme});
        advance();

        keyValuePairs.elements[key] = value;

    } while (match(TokenType::COMMA));

    consume(TokenType::RIGHT_BRACE, "Expected '}' to close the dictionary.");
    emit(Opcode::LOAD_VALUE, peek().line, Value{dictType, keyValuePairs});
    // // Assuming we have a dict type prepared
    // TypePtr dictType = std::make_shared<Type>(TypeTag::Dict);
    // DictValue keyValuePairs;

    // while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
    //     // Parse key
    //     ValuePtr key = std::make_shared<Value>(
    //         Value{std::make_shared<Type>(inferType(peek())),
    //               peek().lexeme}
    //         );
    //         interpolate_string(peek().lexeme);
    //         advance();
    //    // expression();
    //     consume(TokenType::COLON, "Expected ':' after dictionary key.");

    //     // Parse value
    //     ValuePtr dictValue = std::make_shared<Value>(
    //         Value{std::make_shared<Type>(inferType(peek())),
    //               peek().lexeme}
    //         );

    //     keyValuePairs.elements[key] = dictValue;

    //     // Parse each element as an expression
    //     expression();

    //     if (!match(TokenType::COMMA))
    //         break;
    // }

    // consume(TokenType::RIGHT_BRACE, "Expected '}' to close the dictionary.");

    // emit(Opcode::LOAD_VALUE, peek().line, Value{dictType, keyValuePairs});
}

void PackratParser::parallel_statement()
{
    // consume(TokenType::PARALLEL, "Expected 'parallel' keyword.");
    // consume(TokenType::LEFT_BRACE, "Expected '{' after 'parallel'.");
    // std::vector<size_t> taskPositions;

    // while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
    //     if (match(TokenType::IDENTIFIER)) {
    //         Token identifier = previous();

    //         if (identifier.lexeme == "tasks") {
    //             // Parse task list
    //             consume(TokenType::COLON, "Expected ':' after 'tasks'.");
    //             consume(TokenType::LEFT_BRACKET, "Expected '[' to start task list.");

    //             while (!check(TokenType::RIGHT_BRACKET) && !isAtEnd()) {
    //                 consume(TokenType::LEFT_BRACE, "Expected '{' to start task.");
    //                 size_t taskStart = bytecode.size();
    //                 block(); // Parse the task body as a block
    //                 size_t taskEnd = bytecode.size();
    //                 ListValue rangeList;
    //                 rangeList.elements.push_back(
    //                             std::make_shared<Value>(Value{std::make_shared<Type>(TypeTag::Int), taskStart}));
    //                 rangeList.elements.push_back(
    //                     std::make_shared<Value>(Value{std::make_shared<Type>(TypeTag::Int), taskEnd}));
    //                 emit(Opcode::TASK_PARALLEL, peek().line, rangeList);
    //                 taskPositions.push_back(bytecode.size());
    //                 consume(TokenType::RIGHT_BRACE, "Expected '}' to close task.");
    //                 if (match(TokenType::COMMA))
    //                     continue;
    //             }

    //             consume(TokenType::RIGHT_BRACKET, "Expected ']' to close task list.");
    //         } else if (identifier.lexeme == "cores") {
    //             // Parse cores configuration
    //             consume(TokenType::COLON, "Expected ':' after 'cores'.");
    //             Token value = consume(TokenType::IDENTIFIER, "Expected value for 'cores'.");
    //             emit(Opcode::PARALLEL_CORES,
    //                  identifier.line,
    //                  Value{std::make_shared<Type>(TypeTag::Int), value.lexeme});
    //         } else if (identifier.lexeme == "channels") {
    //             // Parse channels configuration
    //             consume(TokenType::COLON, "Expected ':' after 'channels'.");
    //             consume(TokenType::LEFT_BRACE, "Expected '{' for channels configuration.");

    //             while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
    //                 Token channel = peek();
    //                 consume(TokenType::IDENTIFIER, "Expected channel name.");
    //                 consume(TokenType::COLON, "Expected ':' after channel name.");
    //                 Token channelType = peek();
    //                 consume(TokenType::IDENTIFIER, "Expected channel type.");
    //                 emit(Opcode::CHANNEL_DEFINE,
    //                      channel.line,
    //                      Value{channel.lexeme, channelType.lexeme});
    //                 if (match(TokenType::COMMA))
    //                     continue;
    //             }

    //             consume(TokenType::RIGHT_BRACE, "Expected '}' to close channels configuration.");
    //         } else if (identifier.lexeme == "error") {
    //             // Parse error strategy
    //             consume(TokenType::COLON, "Expected ':' after 'error'.");
    //             consume(TokenType::LEFT_BRACE, "Expected '{' for error strategy.");

    //             while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
    //                 Token key = peek();
    //                 consume(TokenType::IDENTIFIER, "Expected error strategy key.");
    //                 consume(TokenType::COLON, "Expected ':' after key.");
    //                 Token value = peek();
    //                 expression(); // Parse value (e.g., `Continue`, 3, `30s`)
    //                 emit(Opcode::ERROR_STRATEGY, key.line, Value{key.lexeme, value.lexeme});
    //                 if (match(TokenType::COMMA))
    //                     continue;
    //             }

    //             consume(TokenType::RIGHT_BRACE, "Expected '}' to close error strategy.");
    //         } else {
    //             std::string msg = "Unexpected identifier in 'parallel' block. identifier:  ";
    //             msg.append(identifier.lexeme);
    //             error(msg);
    //         }
    //     }
    // }

    // consume(TokenType::RIGHT_BRACE, "Expected '}' to close 'parallel' block.");
    // emit(Opcode::PARALLEL_END, peek().line);
}

void PackratParser::concurrent_statement()
{
    // consume(TokenType::CONCURRENT, "Expected 'concurrent' keyword.");
    // consume(TokenType::LEFT_BRACE, "Expected '{' after 'concurrent'.");
    // std::vector<size_t> taskPositions;

    // while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
    //     if (match(TokenType::IDENTIFIER)) {
    //         Token identifier = previous();

    //         if (identifier.lexeme == "worker") {
    //             // Parse worker configuration
    //             consume(TokenType::COLON, "Expected ':' after 'worker'.");
    //             consume(TokenType::FN, "Expected 'fn' for worker function.");
    //             consume(TokenType::LEFT_PAREN, "Expected '(' after 'fn'.");
    //             Token param = peek();
    //             consume(TokenType::IDENTIFIER, "Expected parameter name for worker function.");
    //             consume(TokenType::RIGHT_PAREN, "Expected ')' after parameter.");
    //             consume(TokenType::ARROW, "Expected '->' for worker function return type.");

    //             Token returnType = peek();
    //             consume(TokenType::IDENTIFIER, "Expected return type for worker function.");
    //             consume(TokenType::LEFT_BRACE, "Expected '{' for worker function body.");

    //             size_t functionStart = bytecode.size();
    //             block();
    //             size_t functionEnd = bytecode.size();
    //             emit(Opcode::WORKER_FUNCTION,
    //                  param.line,
    //                  Value{functionStart, functionEnd, param.lexeme, returnType.lexeme});
    //         } else if (identifier.lexeme == "input") {
    //             // Parse input source
    //             consume(TokenType::COLON, "Expected ':' after 'input'.");
    //             Token inputSource = peek();
    //             expression();
    //             emit(Opcode::INPUT_SOURCE, inputSource.line, Value{inputSource.lexeme});
    //         } else if (identifier.lexeme == "output") {
    //             // Parse output channel
    //             consume(TokenType::COLON, "Expected ':' after 'output'.");
    //             Token outputChannel = peek();
    //             expression();
    //             emit(Opcode::OUTPUT_CHANNEL, outputChannel.line, Value{outputChannel.lexeme});
    //         } else {
    //             // Parse task function
    //             if (match(TokenType::LEFT_PAREN)) {
    //                 // Parse arguments for the function call
    //                 std::vector<Value> arguments;
    //                 while (!match(TokenType::RIGHT_PAREN)) {
    //                      Token prev = peek();
    //                     expression();
    //                     arguments.push_back(prev);
    //                     if (match(TokenType::COMMA)) continue;
    //                 }
    //                 emit(Opcode::TASK_CONCURRENT, identifier.line, Value{identifier.lexeme, arguments});
    //             } else {
    //                 std::string msg = "Expected '(' to begin function call or a valid configuration key. identifier:  ";
    //                 msg.append(identifier.lexeme);
    //                 error(msg);

    //             }
    //         }
    //     }

    //     // Optional comma for separating entries
    //     if (match(TokenType::COMMA)) continue;
    // }

    // consume(TokenType::RIGHT_BRACE, "Expected '}' to close 'concurrent' block.");
    // emit(Opcode::CONCURRENT_END, peek().line);
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
     consume(TokenType::IDENTIFIER, "Expected variable name.");

    TokenType assignmentType = TokenType::EQUAL;
    if (match(TokenType::PLUS_EQUAL)) {
        assignmentType = TokenType::PLUS_EQUAL;
    } else if (match(TokenType::MINUS_EQUAL)) {
        assignmentType = TokenType::MINUS_EQUAL;
    } else {
        consume(TokenType::EQUAL, "Expected '=', '+=', or '-=' after variable name.");
    }

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

}

void PackratParser::function_declaration()
{
    Token name = peek();
    consume(TokenType::IDENTIFIER, "Expected function name.");
    consume(TokenType::LEFT_PAREN, "Expected '(' after function name.");

    // Create a temporary bytecode buffer
    std::vector<Instruction> originalBytecode = std::move(bytecode);
    bytecode.clear();  // Clear main bytecode temporarily

    std::vector<ParameterInfo> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token paramName = peek();
            consume(TokenType::IDENTIFIER, "Expected parameter name.");
            TypePtr paramType = typeSystem->NIL_TYPE;
            ValuePtr defaultValue = nullptr;
            bool isOptional = false;

            if (match(TokenType::COLON)) {
                Token typeToken = peek();
                advance();
                paramType = std::make_shared<Type>(stringToType(typeToken.lexeme));
            }

            if (match(TokenType::EQUAL)) {
                isOptional = true;
                Token paramValue = peek();
                advance();
                defaultValue = std::make_shared<Value>(setValue(paramType, paramValue.lexeme));
            } else {
                defaultValue = typeSystem->createValue(paramType);
            }

            parameters.emplace_back(paramName.lexeme, paramType, isOptional, defaultValue);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters.");

    TypePtr returnType = typeSystem->NIL_TYPE;
    if (match(TokenType::COLON)) {
        Token typeToken = peek();
        advance();
        returnType = std::make_shared<Type>(stringToType(typeToken.lexeme));
    }

    // Enter new scope for function body
    enterScope();

    // Record the start of function body in bytecode
    int32_t startPC = bytecode.size();


    // Register parameters as variables in function scope
    for (const auto& param : parameters) {
        declareVariable(Token{TokenType::IDENTIFIER, param.name}, param.type, param.defaultValue);
    }

    // Parse function body
    consume(TokenType::LEFT_BRACE, "Expected '{' before function body.");

    // Store current function body instructions in a temporary vector
    std::vector<Instruction> functionBody;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statement();
    }

    // Store the function body instructions
    functionBody = std::move(bytecode);
    bytecode = std::move(originalBytecode);  // Restore main bytecode

    consume(TokenType::RIGHT_BRACE, "Expected '}' after function block.");

    // Add implicit return if needed
    if (functionBody.empty() || functionBody.back().opcode != Opcode::RETURN) {
        if (returnType->tag != TypeTag::Nil) {
            error("Function must return a value of type " + returnType->toString());
        }
        functionBody.push_back(Instruction{Opcode::RETURN, static_cast<uint32_t>(peek().line)});
    }

    // Register function in the function table
    functions.addFunction(name.lexeme, parameters, returnType, startPC, -1, functionBody);

    // Update the function's endPC
    int32_t endPC = startPC + functionBody.size() - 1;
    functions.updateFunctionEndPC(name.lexeme, endPC);

    exitScope();

    // Emit only the function name for the VM
    emit(Opcode::DEFINE_FUNCTION,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::String), name.lexeme});
}

void PackratParser::function_call(const Token &name)
{
    if (!functions.hasFunction(name.lexeme)) {
        error("Undefined function '" + name.lexeme + "'");
        return;
    }

    auto funcInfo = functions.getFunction(name.lexeme);
    if (!funcInfo) {
        error("Invalid function info for '" + name.lexeme + "'");
        return;
    }

    // Create parameter frame
    emit(Opcode::CREATE_PARAM_FRAME,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::String), name.lexeme});

    // Process arguments
    std::vector<std::string> providedParams;
    size_t argCount = 0;

    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            // Check for named parameter
            bool isNamed = false;
            std::string paramName;

            if (check(TokenType::IDENTIFIER) && checkNext(TokenType::EQUAL)) {
                Token paramToken = peek();
                paramName = paramToken.lexeme;
                advance(); // consume parameter name
                advance(); // consume equals sign
                isNamed = true;

                // Validate parameter exists
                bool found = false;
                for (const auto &param : funcInfo->parameters) {
                    if (param.name == paramName) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    error("Function '" + name.lexeme + "' has no parameter named '" + paramName
                          + "'");
                }
                providedParams.push_back(paramName);
            }

            // Evaluate the argument expression
            expression();

            if (isNamed) {
                emit(Opcode::STORE_PARAM,
                     peek().line,
                     Value{std::make_shared<Type>(TypeTag::String), paramName});
            } else {
                // Positional parameter
                if (argCount >= funcInfo->parameters.size()) {
                    error("Too many arguments provided to function '" + name.lexeme + "'");
                }
                std::string paramName = funcInfo->parameters[argCount].name;
                providedParams.push_back(paramName);
                emit(Opcode::STORE_PARAM,
                     peek().line,
                     Value{std::make_shared<Type>(TypeTag::String), paramName});
            }

            argCount++;
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");

    // Validate required parameters are provided
    for (size_t i = 0; i < funcInfo->parameters.size(); i++) {
        const auto &param = funcInfo->parameters[i];
        if (!param.isOptional) {
            bool provided = false;
            for (const auto &providedParam : providedParams) {
                if (providedParam == param.name) {
                    provided = true;
                    break;
                }
            }
            if (!provided) {
                error("Missing required parameter '" + param.name + "' in call to function '"
                      + name.lexeme + "'");
            }
        }
    }

    // Add default values for non-provided optional parameters
    for (const auto &param : funcInfo->parameters) {
        bool provided = false;
        for (const auto &providedParam : providedParams) {
            if (providedParam == param.name) {
                provided = true;
                break;
            }
        }

        if (!provided && param.isOptional) {
            // Load and store default value
            Value defaultVal = *param.defaultValue;
            emit(Opcode::LOAD_CONST, peek().line, std::move(defaultVal));
            emit(Opcode::STORE_PARAM,
                 peek().line,
                 Value{std::make_shared<Type>(TypeTag::String), param.name});
        }
    }

    // Invoke function
    emit(Opcode::INVOKE_FUNCTION,
         peek().line,
         Value{std::make_shared<Type>(TypeTag::String), name.lexeme});

    // Cleanup parameter frame
    emit(Opcode::POP_PARAM_FRAME, peek().line);
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
    range_expression();

    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) || match(TokenType::LESS)
           || match(TokenType::LESS_EQUAL)) {
        TokenType operatorType = previous().type;
        range_expression();

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

    while (match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::MODULUS)) {
        TokenType operatorType = previous().type;
        unary_expression();

        if (operatorType == TokenType::STAR) {
            emit(Opcode::MULTIPLY, peek().line);
        }else if(operatorType == TokenType::MODULUS) {
            emit(Opcode::MODULUS, peek().line);
        }else {
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
    } else if (match(TokenType::COMMA)) {
        advance();
    }  else  if (match(TokenType::LEFT_BRACKET) || match(TokenType::RIGHT_BRACKET) ) {
        statement();
    } else if (match(TokenType::LEFT_BRACE)) {
        statement();
        }
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

void PackratParser::range_function()
{
    match(TokenType::RANGE);
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'range'.");
    expression(); // Parse the beginning value
    consume(TokenType::COMMA, "Expected ',' after beginning value.");
    expression(); // Parse the ending value

    // Optional step value (default to 1 if not provided)
    if (match(TokenType::COMMA)) {
        expression(); // Parse the step value
    } else {
        // Emit a constant step value of 1
        emit(Opcode::LOAD_CONST, peek().line, setValue(std::make_shared<Type>(TypeTag::Int64), "1"));
    }

    consume(TokenType::RIGHT_PAREN, "Expected ')' after range arguments.");

    // Emit a custom opcode to handle the range iteration in the VM
    emit(Opcode::MAKE_RANGE,
         peek().line,
         Value{}); // No specific value, just to signal range construction
}

void PackratParser::range_expression()
{
    additive_expression();
    // Handle the simple form: begin..end

    if (match(TokenType::DOT_DOT)) { // Match '..' for range
        // Parse the end expression
        additive_expression();

        // Optional step expression if we use something like '..<step>'
        if (match(TokenType::DOT_DOT)) {
            additive_expression();
        } else {
            // Emit a constant step value of 1
            emit(Opcode::LOAD_CONST,
                 peek().line,
                 setValue(std::make_shared<Type>(TypeTag::Int64), "1"));
        }
        // Emit a custom opcode to handle the range iteration in the VM
        emit(Opcode::MAKE_RANGE,
             peek().line,
             Value{}); // No specific value, just to signal range construction
    }
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
   // instruction.debug();
    bytecode.push_back(instruction);
    return instruction;
}

Instruction PackratParser::emit(Opcode opcode, uint32_t lineNumber, Value &&value)
{
    ValuePtr valuePtr = std::make_shared<Value>(std::move(value));
    Instruction instruction(opcode, lineNumber, valuePtr);
  //  instruction.debug();
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
    // if (defaultValue) {

    // }
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
            ListValue listValue;
            if (!input.empty() && input != "[]") {
                std::string trimmedInput = input.substr(1, input.length() - 2); // Remove brackets
                std::istringstream iss(trimmedInput);
                std::string item;
                while (std::getline(iss, item, ',')) {
                    // Trim whitespace
                    item.erase(0, item.find_first_not_of(" "));
                    item.erase(item.find_last_not_of(" ") + 1);

                    // Recursively convert each list item
                 //listValue.elements.push_back(setValue(std::make_shared<Type>(type->tag), item));
                listValue.elements.push_back(
                        std::make_shared<Value>(setValue(std::get_if<ListType>(&type->extra)->elementType, item))
                        );
                }
            }
            value.data = listValue;
        }
        break;
    case TypeTag::Dict:
        // Assuming input is in the format "key1:value1,key2:value2"
        {
            DictValue dictValue;
            if (!input.empty() && input != "{}") {
                std::string trimmedInput = input.substr(1, input.length() - 2); // Remove braces
                std::istringstream iss(trimmedInput);
                std::string pair;
                while (std::getline(iss, pair, ',')) {
                    size_t colonPos = pair.find(':');
                    if (colonPos != std::string::npos) {
                        std::string key = pair.substr(0, colonPos);
                        std::string val = pair.substr(colonPos + 1);

                        // Trim whitespace
                        key.erase(0, key.find_first_not_of(" "));
                        key.erase(key.find_last_not_of(" ") + 1);
                        val.erase(0, val.find_first_not_of(" "));
                        val.erase(val.find_last_not_of(" ") + 1);

                        // Get key and value types from the Dict type
                        auto* dictType = std::get_if<DictType>(&type->extra);
                        if (dictType) {
                            ValuePtr keyValue = std::make_shared<Value>(setValue(dictType->keyType, key));
                            ValuePtr valValue = std::make_shared<Value>(setValue(dictType->valueType, val));
                            dictValue.elements[keyValue] = valValue;
                        }
                    }
                }
            }
            value.data = dictValue;
        }
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

bool PackratParser::checkNext(TokenType type)
{
    if (isAtEnd()) {
        return false;
    }
    return peekNext().type == type;
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
