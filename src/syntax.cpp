////syntax.cpp
//#include "syntax.hh"
//#include "debugger.hh"
//#include "scanner.hh"
//#include "opcodes.hh"

//void Syntax::parseFunctionDeclaration(Scanner &scanner)
//{
//    // Parse function declaration syntax
//    // Example:
//    // fn greeting(name: str, date: str? = None): str {
//    //     var message: str = "Hello {name}";
//    //     if let date_str = date {
//    //         message += ", today's date is {date_str}.toDate()";
//    //     }
//    //     return message;
//    // }
//    advance(scanner); // Consume 'fn' token
//    if (!match(scanner, TokenType::IDENTIFIER)) {
//        error("Expected function name after 'fn'.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume function name token

//    if (!match(scanner, TokenType::LEFT_PAREN)) {
//        error("Expected '(' after function name.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '(' token

//    // Parse function parameters
//    if (!match(scanner, TokenType::RIGHT_PAREN)) {
//        // Parse parameter list
//        while (true) {
//            if (!match(scanner, TokenType::IDENTIFIER)) {
//                error("Expected parameter name.", scanner.line, scanner.current);
//            }
//            advance(scanner); // Consume parameter name token

//            if (!match(scanner, TokenType::COLON)) {
//                error("Expected ':' after parameter name.", scanner.line, scanner.current);
//            }
//            advance(scanner); // Consume ':' token

//            if (!match(scanner, TokenType::IDENTIFIER)) {
//                error("Expected parameter type.", scanner.line, scanner.current);
//            }
//            advance(scanner); // Consume parameter type token

//            // Check for default value
//            if (match(scanner, TokenType::EQUAL)) {
//                advance(scanner);         // Consume '=' token
//                parseExpression(scanner); // Parse default value expression
//            }

//            // Check for optional parameter
//            if (match(scanner, TokenType::QUESTION)) {
//                advance(scanner); // Consume '?' token
//            }

//            // Check for parameter list continuation or end
//            if (match(scanner, TokenType::COMMA)) {
//                advance(scanner); // Consume ',' token
//            } else if (match(scanner, TokenType::RIGHT_PAREN)) {
//                advance(scanner); // Consume ')' token
//                break;            // End of parameter list
//            } else {
//                error("Expected ',' or ')' after parameter.", scanner.line, scanner.current);
//            }
//        }
//    } else {
//        advance(scanner); // Consume ')' token
//    }

//    // Check for return type declaration
//    if (match(scanner, TokenType::COLON)) {
//        advance(scanner); // Consume ':' token
//        if (!match(scanner, TokenType::IDENTIFIER)) {
//            error("Expected return type after ':' in function declaration.",
//                  scanner.line,
//                  scanner.current);
//        }
//        advance(scanner); // Consume return type token
//    }

//    // Parse function body
//    if (!match(scanner, TokenType::LEFT_BRACE)) {
//        error("Expected '{' before function body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '{' token
//    // Parse function body statements
//    while (!match(scanner, TokenType::RIGHT_BRACE) && !match(scanner, TokenType::EOF_TOKEN)) {
//        parseExpression(scanner); // Parse statements within the function body
//    }
//    if (!match(scanner, TokenType::RIGHT_BRACE)) {
//        error("Expected '}' after function body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '}' token
//}

////void Syntax::parseForRange(Scanner &scanner)
////{
////    // Parse for loop syntax with range
////    // Example:
////    // for (var i in range(10)) {
////    //     ...
////    // }
////    advance(scanner); // Consume 'for' token
////    if (!match(scanner, TokenType::LEFT_PAREN)) {
////        error("Expected '(' after 'for'.", scanner.line, scanner.current);
////    }
////    advance(scanner); // Consume '(' token

////    // Parse variable declaration and range function call
////    parseVariableDeclaration(scanner); // Parse variable declaration
////    if (!match(scanner, TokenType::IN)) {
////        error("Expected 'in' after variable declaration in for loop with range.",
////              scanner.line,
////              scanner.current);
////    }
////    advance(scanner); // Consume 'in' token
////    if (!match(scanner, TokenType::RANGE)) {
////        error("Expected 'range' function call in for loop with range.",
////              scanner.line,
////              scanner.current);
////    }
////    advance(scanner); // Consume 'range' token
////    if (!match(scanner, TokenType::LEFT_PAREN)) {
////        error("Expected '(' after 'range'.", scanner.line, scanner.current);
////    }
////    advance(scanner);         // Consume '(' token
////    parseExpression(scanner); // Parse expression within range function call
////    if (!match(scanner, TokenType::RIGHT_PAREN)) {
////        error("Expected ')' after range function call.", scanner.line, scanner.current);
////    }
////    advance(scanner); // Consume ')' token

////    if (!match(scanner, TokenType::RIGHT_PAREN)) {
////        error("Expected ')' after range function call.", scanner.line, scanner.current);
////    }
////    advance(scanner); // Consume ')' token

////    // Parse loop body
////    if (!match(scanner, TokenType::LEFT_BRACE)) {
////        error("Expected '{' before loop body.", scanner.line, scanner.current);
////    }
////    advance(scanner); // Consume '{' token
////    while (!match(scanner, TokenType::RIGHT_BRACE) && !match(scanner, TokenType::EOF_TOKEN)) {
////        parseExpression(scanner); // Parse statements within the loop body
////    }
////    if (!match(scanner, TokenType::RIGHT_BRACE)) {
////        error("Expected '}' after loop body.", scanner.line, scanner.current);
////    }
////    advance(scanner); // Consume '}' token
////}

//void Syntax::parseForLoop(Scanner &scanner)
//{
//    // Parse for loop syntax
//    // Example:
//    // for (var i = 0; i < 5; i++) {
//    //     ...
//    // }
//    advance(scanner); // Consume 'for' token
//    if (!match(scanner, TokenType::LEFT_PAREN)) {
//        error("Expected '(' after 'for'.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '(' token

//    // Parse initialization
//    parseVariableDeclaration(scanner); // Parse variable declaration

//    // Parse condition
//    if (!match(scanner, TokenType::SEMICOLON)) {
//        error("Expected ';' after loop initialization.", scanner.line, scanner.current);
//    }
//    advance(scanner);         // Consume ';' token
//    parseExpression(scanner); // Parse loop condition

//    // Parse increment
//    if (!match(scanner, TokenType::SEMICOLON)) {
//        error("Expected ';' after loop condition.", scanner.line, scanner.current);
//    }
//    advance(scanner);         // Consume ';' token
//    parseExpression(scanner); // Parse loop increment

//    if (!match(scanner, TokenType::RIGHT_PAREN)) {
//        error("Expected ')' after loop increment.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume ')' token

//    // Parse loop body
//    if (!match(scanner, TokenType::LEFT_BRACE)) {
//        error("Expected '{' before loop body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '{' token
//    while (!match(scanner, TokenType::RIGHT_BRACE) && !match(scanner, TokenType::EOF_TOKEN)) {
//        parseExpression(scanner); // Parse statements within the loop body
//    }
//    if (!match(scanner, TokenType::RIGHT_BRACE)) {
//        error("Expected '}' after loop body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '}' token
//}

//void Syntax::parseWhileLoop(Scanner &scanner)
//{
//    // Parse while loop syntax
//    // Example:
//    // while (condition) {
//    //     ...
//    // }
//    advance(scanner); // Consume 'while' token
//    if (!match(scanner, TokenType::LEFT_PAREN)) {
//        error("Expected '(' after 'while'.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '(' token

//    // Parse condition
//    parseExpression(scanner); // Parse while loop condition

//    if (!match(scanner, TokenType::RIGHT_PAREN)) {
//        error("Expected ')' after condition.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume ')' token

//    // Parse loop body
//    if (!match(scanner, TokenType::LEFT_BRACE)) {
//        error("Expected '{' before loop body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '{' token
//    while (!match(scanner, TokenType::RIGHT_BRACE) && !match(scanner, TokenType::EOF_TOKEN)) {
//        parseExpression(scanner); // Parse statements within the loop body
//    }
//    if (!match(scanner, TokenType::RIGHT_BRACE)) {
//        error("Expected '}' after loop body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '}' token
//}

//void Syntax::parseConditional(Scanner &scanner)
//{
//    // Parse conditional statement syntax
//    // Example:
//    // if (condition) {
//    //     // code block
//    // } else if (condition) {
//    //     // code block
//    // } else {
//    //     // code block
//    // }
//    advance(scanner); // Consume 'if' token

//    if (!match(scanner, TokenType::LEFT_PAREN)) {
//        error("Expected '(' after 'if'.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '(' token

//    parseExpression(scanner); // Parse condition expression

//    if (!match(scanner, TokenType::RIGHT_PAREN)) {
//        error("Expected ')' after condition.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume ')' token

//    if (!match(scanner, TokenType::LEFT_BRACE)) {
//        error("Expected '{' before if statement body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '{' token

//    // Parse if statement body
//    while (!match(scanner, TokenType::RIGHT_BRACE) && !match(scanner, TokenType::EOF_TOKEN)) {
//        // Parse statements within the if statement body
//        parseExpression(scanner);
//    }

//    if (!match(scanner, TokenType::RIGHT_BRACE)) {
//        error("Expected '}' after if statement body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '}' token

//    // Check for 'else if' or 'else' block
//    if (match(scanner, TokenType::ELSE)) {
//        advance(scanner); // Consume 'else' token
//        if (match(scanner, TokenType::IF)) {
//            parseConditional(scanner); // Parse 'else if' block recursively
//        } else {
//            if (!match(scanner, TokenType::LEFT_BRACE)) {
//                error("Expected '{' before else statement body.", scanner.line, scanner.current);
//            }
//            advance(scanner); // Consume '{' token

//            // Parse else statement body
//            while (!match(scanner, TokenType::RIGHT_BRACE)
//                   && !match(scanner, TokenType::EOF_TOKEN)) {
//                // Parse statements within the else statement body
//                parseExpression(scanner);
//            }

//            if (!match(scanner, TokenType::RIGHT_BRACE)) {
//                error("Expected '}' after else statement body.", scanner.line, scanner.current);
//            }
//            advance(scanner); // Consume '}' token
//        }
//    }
//}

//void Syntax::parseClassDeclaration(Scanner &scanner)
//{
//    // Parse class declaration syntax
//    // Example:
//    // class MyClass {
//    //     // class members
//    // }
//    advance(scanner); // Consume 'class' token

//    // Parse class name (identifier)
//    parseIdentifier(scanner); // Parse class name

//    if (!match(scanner, TokenType::LEFT_BRACE)) {
//        error("Expected '{' before class body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '{' token

//    // Parse class members (variables, methods, etc.)
//    while (!match(scanner, TokenType::RIGHT_BRACE) && !match(scanner, TokenType::EOF_TOKEN)) {
//        // You can implement logic to parse class members here
//        // For example, call parseVariableDeclaration or parseFunctionDeclaration
//        // Ensure to handle different class members appropriately
//        advance(scanner); // Placeholder for parsing class members
//    }

//    if (!match(scanner, TokenType::RIGHT_BRACE)) {
//        error("Expected '}' after class body.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '}' token
//}

//void Syntax::parseVariableDeclaration(Scanner &scanner)
//{
//    // Parse variable declaration syntax
//    // Example:
//    // var myVar: int = 10;
//    advance(scanner);         // Consume 'var' token
//    parseIdentifier(scanner); // Parse variable name
//    advance(scanner);         // Consume ':' token
//    parseType(scanner);      // Parse variable type
//    std::string variableName = scanner.getToken().lexeme;
//    emit(Opcode::DECLARE_VARIABLE, scanner.getLine(), variableName);// Emit the opcode to declare the variable

//    if (match(scanner, TokenType::EQUAL)) {
//        advance(scanner);         // Consume '=' token
//        parseExpression(scanner); // Parse expression
//    }
//}

//void Syntax::parseAssignment(Scanner &scanner)
//{
//    // Parse assignment syntax
//    // Example:
//    // myVar = 20;
//    parseIdentifier(scanner); // Parse variable name
//    advance(scanner);         // Consume '=' token
//    parseExpression(scanner); // Parse expression
//}

//void Syntax::parseExpression(Scanner &scanner)
//{
//    if (match(scanner, TokenType::LEFT_PAREN)) {
//        advance(scanner); // Consume '(' token
//        // Parse nested expression within parentheses
//        parseExpression(scanner);
//        if (!match(scanner, TokenType::RIGHT_PAREN)) {
//            error("Expected ')' after expression.", scanner.line, scanner.current);
//        }
//        advance(scanner); // Consume ')' token
//    } else if (match(scanner, TokenType::IDENTIFIER)) {
//        // Parse function calls
//        parseIdentifier(scanner); // Parse function name
//        if (match(scanner, TokenType::LEFT_PAREN)) {
//            advance(scanner); // Consume '(' token
//            if (!match(scanner, TokenType::RIGHT_PAREN)) {
//                parseArguments(scanner); // Parse function arguments
//            }
//            if (!match(scanner, TokenType::RIGHT_PAREN)) {
//                error("Expected ')' after function arguments in expression.",
//                    scanner.line,
//                    scanner.current);
//            }
//            advance(scanner); // Consume ')' token
//        }
//    } else if (match(scanner, TokenType::STRING)) {
//        // Emit the opcode to store the value in the variable
//        std::string value = scanner.getToken().lexeme;
//        emit(Opcode::STORE_STR, scanner.getLine(), value);
//        // Parse string literals
//        advance(scanner); // Consume string token
//    } else if (match(scanner, TokenType::NUMBER)) {
//        std::string value = scanner.getToken().lexeme;
//        emit(Opcode::STORE_VALUE, scanner.getLine(), value);
//        // Parse numeric literals
//        advance(scanner); // Consume number token
//    } else {
//        error("Invalid expression.", scanner.line, scanner.current);
//    }
//}

//void Syntax::parseAttempt(Scanner &scanner)
//{
//    advance(scanner); // Consume 'attempt' token
//    // Parse attempt block
//    while (!match(scanner, TokenType::HANDLE) && !match(scanner, TokenType::EOF_TOKEN)) {
//        parseExpression(scanner); // Parse statements within the attempt block
//    }
//    if (match(scanner, TokenType::HANDLE)) {
//        advance(scanner); // Consume 'handle' token
//        // Parse handle block
//        while (!match(scanner, TokenType::EOF_TOKEN)) {
//            parseExpression(scanner); // Parse statements within the handle block
//        }
//    } else {
//        error("Expected 'handle' after 'attempt'.", scanner.line, scanner.current);
//    }
//}

//void Syntax::parseString(Scanner &scanner)
//{
//    // Check if the next token is an identifier (variable name)
//    if (!match(scanner, TokenType::IDENTIFIER)) {
//        error("Expected identifier in string declaration");
//        return;
//    }

//    // Parse the variable name
//    std::string variableName = scanner.getToken().lexeme;

//    // Consume the identifier token
//    advance(scanner);

//    // Check if the next token is an assignment operator
//    if (!match(scanner, TokenType::EQUAL)) {
//        error("Expected assignment operator in string declaration");
//        return;
//    }

//    // Consume the assignment operator token
//    advance(scanner);

//    // Check if the next token is a string literal
//    if (!match(scanner, TokenType::STRING)) {
//        error("Expected string literal in string declaration");
//        return;
//    }

//    // Parse the string literal
//    std::string stringValue = scanner.getToken().lexeme;

//    // Consume the string literal token
//    advance(scanner);

//    // Handle the assignment of the string value to the variable
//    // For example, you can store this information in your program's data structures
//    // Here, you can generate intermediate code or construct an abstract syntax tree (AST)

//    // Example: Constructing intermediate code
//    // intermediateCodeGenerator.generateAssignment(variableName, stringValue);

//    // Example: Constructing AST nodes
//    // ast.addNode(AssignmentNode(variableName, StringLiteralNode(stringValue)));

//    // Optionally, you can perform additional validations or actions based on your language's syntax rules

//    // Print debugging information (optional)
//    std::cout << "Parsed string declaration: " << variableName << " = " << stringValue << std::endl;
//}

//void Syntax::parseConcurrent(Scanner &scanner)
//{
//    advance(scanner); // Consume 'concurrent' token
//    // Parse concurrent block
//    while (!match(scanner, TokenType::RIGHT_BRACE) && !match(scanner, TokenType::EOF_TOKEN)) {
//        parseExpression(scanner); // Parse statements within the concurrent block
//    }
//    if (!match(scanner, TokenType::RIGHT_BRACE)) {
//        error("Expected '}' after 'concurrent'.", scanner.line, scanner.current);
//    } else {
//        advance(scanner); // Consume '}' token
//    }
//}

//void Syntax::parseParallel(Scanner &scanner)
//{
//    advance(scanner); // Consume 'parallel' token
//    // Parse parallel block
//    while (!match(scanner, TokenType::RIGHT_BRACE) && !match(scanner, TokenType::EOF_TOKEN)) {
//        parseExpression(scanner); // Parse statements within the parallel block
//    }
//    if (!match(scanner, TokenType::RIGHT_BRACE)) {
//        error("Expected '}' after 'parallel'.", scanner.line, scanner.current);
//    } else {
//        advance(scanner); // Consume '}' token
//    }
//}

//void Syntax::parseAwait(Scanner &scanner)
//{
//    advance(scanner);         // Consume 'await' token
//    parseExpression(scanner); // Parse asynchronous task expression
//}

//void Syntax::parseAsync(Scanner &scanner)
//{
//    advance(scanner);                  // Consume 'async' token
//    parseFunctionDeclaration(scanner); // Parse asynchronous task function declaration
//}

////void Syntax::parseBlock(Scanner &scanner)
////{
////    consume(TokenType::LEFT_BRACE, "Expected '{' to start block.");

////    while (!match(scanner, TokenType::RIGHT_BRACE) && !parser.match(TokenType::EOF_TOKEN)) {
////        statement();
////    }

////    consume(TokenType::RIGHT_BRACE, "Expected '}' to end block.");
////}

//void Syntax::parsePatternMatching(Scanner &scanner)
//{
//    // Parse pattern matching syntax
//    // Example:
//    // match(value){
//    //     int:
//    //         print("The value is an integer");
//    //     str:
//    //         print("The value is a string");
//    //     list:
//    //         print("The value is a list");
//    //     dict:
//    //         print("The value is a dictionary");
//    //     _:
//    //         print("Unknown type");
//    // }
//    advance(scanner); // Consume 'match' token

//    // Parse value to match
//    parseExpression(scanner); // Parse value expression

//    // Parse match cases
//    while (!match(scanner, TokenType::RIGHT_BRACE) && !match(scanner, TokenType::EOF_TOKEN)) {
//        parseMatchCase(scanner); // Parse individual match case
//    }

//    if (!match(scanner, TokenType::RIGHT_BRACE)) {
//        error("Expected '}' after match cases.", scanner.line, scanner.current);
//    }
//    advance(scanner); // Consume '}' token
//}

//void Syntax::parseMatchCase(Scanner &scanner)
//{
//    // Parse individual match case
//    // Example:
//    // 42:
//    //     print("The value is the number 42");
//    // Or:
//    // "hello":
//    //     print("The value is the string 'hello'");
//    // Or:
//    // some_identifier:
//    //     print("The value is an identifier 'some_identifier'");
//    // Or:
//    // _:
//    //     print("Unknown value");

//    if (match(scanner, TokenType::NUMBER) || match(scanner, TokenType::STRING)
//        || match(scanner, TokenType::IDENTIFIER)) {
//        advance(scanner); // Consume number, string, or identifier token
//    } else if (match(scanner, TokenType::DEFAULT)) {
//        advance(scanner); // Consume underscore token
//    } else {
//        error("Expected pattern (number, string, identifier, or '_').",
//            scanner.getLine(),
//            scanner.getCurrent());
//        return;
//    }

//    if (!match(scanner, TokenType::COLON)) {
//        error("Expected ':' after pattern.", scanner.getLine(), scanner.getCurrent());
//        return;
//    }
//    advance(scanner); // Consume ':' token

//    parseExpression(scanner); // Parse case body
//}

//void Syntax::ternary(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse condition
//    advance(scanner);         // Consume '?' token
//    parseExpression(scanner); // Parse expression for true branch
//    advance(scanner);         // Consume ':' token
//    parseExpression(scanner); // Parse expression for false branch
//}

//void Syntax::logicalOr(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse left operand
//    advance(scanner);         // Consume 'or' token
//    parseExpression(scanner); // Parse right operand
//}

//void Syntax::logicalAnd(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse left operand
//    advance(scanner);         // Consume 'and' token
//    parseExpression(scanner); // Parse right operand
//}

//void Syntax::equality(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse left operand
//    advance(scanner);         // Consume '==' token
//    parseExpression(scanner); // Parse right operand
//}

//void Syntax::comparison(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse left operand
//    advance(scanner);         // Consume comparison token
//    parseExpression(scanner); // Parse right operand
//}

//void Syntax::addition(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse left operand
//    advance(scanner);         // Consume '+' token
//    parseExpression(scanner); // Parse right operand
//}

//void Syntax::subtraction(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse left operand
//    advance(scanner);         // Consume '-' token
//    parseExpression(scanner); // Parse right operand
//}

//void Syntax::multiplication(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse left operand
//    advance(scanner);         // Consume '*' token
//    parseExpression(scanner); // Parse right operand
//}

//void Syntax::division(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse left operand
//    advance(scanner);         // Consume '/' token
//    parseExpression(scanner); // Parse right operand
//}

//void Syntax::modulus(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse left operand
//    advance(scanner);         // Consume '%' token
//    parseExpression(scanner); // Parse right operand
//}

//void Syntax::unary(Scanner &scanner)
//{
//    // Parse unary operation syntax
//    // Example:
//    // var result = -value;
//    if (match(scanner, TokenType::BANG) || match(scanner, TokenType::MINUS)) {
//        advance(scanner);         // Consume unary operator
//        parseExpression(scanner); // Parse operand
//    } else {
//        primary(scanner); // Parse primary expression
//    }
//}

//void Syntax::primary(Scanner &scanner){
//    // Parse primary expression syntax
//    // Example:
//    // var value = (1 + 2) * 3;
//    if (match(scanner, TokenType::LEFT_PAREN)) {
//        advance(scanner);         // Consume '(' token
//        parseExpression(scanner); // Parse expression within parentheses
//        advance(scanner);         // Consume ')' token
//    } else if (match(scanner, TokenType::IDENTIFIER) || match(scanner, TokenType::NUMBER)
//               || match(scanner, TokenType::STRING)) {
//        advance(scanner); // Consume identifier, number, or string token
//    } else {
//        error("Expected primary expression.", scanner.line, scanner.current);
//    }
//}

//void Syntax::parseIdentifier(Scanner &scanner){
//    if (match(scanner, TokenType::IDENTIFIER)) {
//        std::string value = scanner.getToken().lexeme;
//        emit(Opcode::LOAD_VARIABLE, scanner.getLine(), value);
//        advance(scanner); // Consume identifier token
//    } else {
//        error("Expected an identifier.", scanner.line, scanner.current);
//    }
//}

//void Syntax::parseType(Scanner &scanner)
//{
//    if (match(scanner, TokenType::INT_TYPE) || match(scanner, TokenType::FLOAT_TYPE)
//        || match(scanner, TokenType::STR_TYPE) || match(scanner, TokenType::BOOL_TYPE)
//        || match(scanner, TokenType::USER_TYPE) || match(scanner, TokenType::FUNCTION_TYPE)
//        || match(scanner, TokenType::LIST_TYPE) || match(scanner, TokenType::DICT_TYPE)
//        || match(scanner, TokenType::ARRAY_TYPE) || match(scanner, TokenType::ENUM_TYPE)) {
//        advance(scanner); // Consume type token
//    } else {
//        error("Expected a valid type.", scanner.line, scanner.current);
//    }
//}

//void Syntax::parseArguments(Scanner &scanner)
//{
//    parseExpression(scanner); // Parse first argument
//    while (match(scanner, TokenType::COMMA)) {
//        advance(scanner);         // Consume ',' token
//        parseExpression(scanner); // Parse next argument
//    }
//}

//void Syntax::parsePrintStatement(Scanner &scanner)
//{
//    // Consume the print keyword
//    match(scanner, TokenType::PRINT);

//    // Consume the left parenthesis
//    match(scanner, TokenType::LEFT_PAREN);

//    // Parse the print statement string
//    if (match(scanner, TokenType::STRING)) {
//        std::string printString = scanner.getPrevToken().lexeme;

//        // Perform string interpolation if needed
//        //        while (match(scanner, TokenType::IDENTIFIER)) {
//        //            std::string identifier = scanner.getPrevToken().lexeme;
//        //            std::string replacement = getVariableValue(identifier); // Retrieve value for identifier
//        //            printString = replaceSubstring(printString, "{" + identifier + "}", replacement);
//        //        }

//        // Print the resulting string
//        std::cout << printString;
//    }

//    // Consume the right parenthesis
//    match(scanner, TokenType::RIGHT_PAREN);

//    // Consume the semicolon
//    match(scanner, TokenType::SEMICOLON);
//    //    emit(Opcode::PRINT, scanner.getLine(), printString);
//}

//void Syntax::parseReturnStatement(Scanner &scanner)
//{
//    // Parse return statement syntax
//    // Example syntax: return value;
//    // Implement according to your language's return statement rules

//    // Consume the return keyword
//    match(scanner, TokenType::RETURN);

//    // Parse the expression following the return keyword
//    parseExpression(scanner);

//    // Consume the semicolon
//    match(scanner, TokenType::SEMICOLON);
//}

//char Syntax::advance(Scanner &scanner)
//{
//    return scanner.advance();
//}

//bool Syntax::match(Scanner &scanner, TokenType expected)
//{
//    if (scanner.isAtEnd())
//        return false;
//    //    return scanner.peek().type == expected;
//    if (!match(scanner, expected))
//        return false;
//    scanner.advance(); // Consume the token
//    return true;
//}

//void Syntax::error(const std::string &message, int line, int start)
//{
//    Debugger::error(message, line, start, InterpretationStage::SYNTAX);
//}

//void Syntax::emit(Opcode op, uint32_t lineNumber, int32_t intValue){
//    Bytecode bytecode;
//    if (intValue > 0) {
//        bytecode.push_back(Instruction{op, lineNumber, intValue});
//    }
//}

//void Syntax::emit(Opcode op, uint32_t lineNumber, float floatValue){
//    Bytecode bytecode;
//    if (floatValue > 0.0f) {
//        bytecode.push_back(Instruction{op, lineNumber, floatValue});
//    }
//}

//void Syntax::emit(Opcode op, uint32_t lineNumber, bool boolValue){
//    Bytecode bytecode;
//    if (boolValue == true) {
//        bytecode.push_back(Instruction{op, lineNumber, boolValue});
//    }
//}

//void Syntax::emit(Opcode op, uint32_t lineNumber, const std::string &stringValue){
//    Bytecode bytecode;
//    if (stringValue != "") {
//        bytecode.push_back(Instruction{op, lineNumber, stringValue});
//    }
//}

//void Syntax::emit(Opcode op, uint32_t lineNumber, bool boolValue){
//    Bytecode bytecode;
//    if (boolValue == true) {
//        bytecode.push_back(Instruction{op, lineNumber, boolValue});
//    }
//}

//void Syntax::emit(Opcode op, uint32_t lineNumber){
//    Bytecode bytecode;
//    if (stringValue != "") {
//        bytecode.push_back(Instruction{op, lineNumber});
//    }
//}
