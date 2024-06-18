// #include "gtest/gtest.h"
// #include "scanner.hh"

// TEST(ScannerTest, BasicTokens) {
//     std::string source = "() {} [] , . - + ? : ; * ! = < > / % _";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 20);

//     ASSERT_EQ(tokens[0].type, TokenType::LEFT_PAREN);
//     ASSERT_EQ(tokens[1].type, TokenType::RIGHT_PAREN);
//     ASSERT_EQ(tokens[2].type, TokenType::LEFT_BRACE);
//     ASSERT_EQ(tokens[3].type, TokenType::RIGHT_BRACE);
//     ASSERT_EQ(tokens[4].type, TokenType::LEFT_BRACKET);
//     ASSERT_EQ(tokens[5].type, TokenType::RIGHT_BRACKET);
//     ASSERT_EQ(tokens[6].type, TokenType::COMMA);
//     ASSERT_EQ(tokens[7].type, TokenType::DOT);
//     ASSERT_EQ(tokens[8].type, TokenType::MINUS);
//     ASSERT_EQ(tokens[9].type, TokenType::PLUS);
//     ASSERT_EQ(tokens[10].type, TokenType::QUESTION);
//     ASSERT_EQ(tokens[11].type, TokenType::COLON);
//     ASSERT_EQ(tokens[12].type, TokenType::SEMICOLON);
//     ASSERT_EQ(tokens[13].type, TokenType::STAR);
//     ASSERT_EQ(tokens[14].type, TokenType::BANG);
//     ASSERT_EQ(tokens[15].type, TokenType::EQUAL);
//     ASSERT_EQ(tokens[16].type, TokenType::LESS);
//     ASSERT_EQ(tokens[17].type, TokenType::GREATER);
//     ASSERT_EQ(tokens[18].type, TokenType::SLASH);
//     ASSERT_EQ(tokens[19].type, TokenType::MODULUS);
// }

// TEST(ScannerTest, TwoCharacterTokens) {
//     std::string source = "!= == <= >= -= +=";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 7);

//     ASSERT_EQ(tokens[0].type, TokenType::BANG_EQUAL);
//     ASSERT_EQ(tokens[1].type, TokenType::EQUAL_EQUAL);
//     ASSERT_EQ(tokens[2].type, TokenType::LESS_EQUAL);
//     ASSERT_EQ(tokens[3].type, TokenType::GREATER_EQUAL);
//     ASSERT_EQ(tokens[4].type, TokenType::MINUS_EQUAL);
//     ASSERT_EQ(tokens[5].type, TokenType::PLUS_EQUAL);
// }

// TEST(ScannerTest, StringLiterals) {
//     std::string source = "\"Hello, world!\" 'single quote'";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 3);

//     ASSERT_EQ(tokens[0].type, TokenType::STRING);
//     ASSERT_EQ(tokens[0].lexeme, "Hello, world!");
//     ASSERT_EQ(tokens[1].type, TokenType::STRING);
//     ASSERT_EQ(tokens[1].lexeme, "single quote");
// }

// TEST(ScannerTest, NumberLiterals) {
//     std::string source = "123 4.56 7.89e10";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 3);

//     ASSERT_EQ(tokens[0].type, TokenType::NUMBER);
//     ASSERT_EQ(tokens[0].lexeme, "123");
//     ASSERT_EQ(tokens[1].type, TokenType::NUMBER);
//     ASSERT_EQ(tokens[1].lexeme, "4.56");
//     ASSERT_EQ(tokens[2].type, TokenType::NUMBER);
//     ASSERT_EQ(tokens[2].lexeme, "7.89e10");
// }

// TEST(ScannerTest, IdentifiersAndKeywords) {
//     std::string source = "and class else false for fn if nil or print return super this true var while int float str bool list dict array enum attempt handle parallel concurrent async await import";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 32);

//     ASSERT_EQ(tokens[0].type, TokenType::AND);
//     ASSERT_EQ(tokens[1].type, TokenType::CLASS);
//     ASSERT_EQ(tokens[2].type, TokenType::ELSE);
//     ASSERT_EQ(tokens[3].type, TokenType::FALSE);
//     ASSERT_EQ(tokens[4].type, TokenType::FOR);
//     ASSERT_EQ(tokens[5].type, TokenType::FN);
//     ASSERT_EQ(tokens[6].type, TokenType::IF);
//     ASSERT_EQ(tokens[7].type, TokenType::NIL);
//     ASSERT_EQ(tokens[8].type, TokenType::OR);
//     ASSERT_EQ(tokens[9].type, TokenType::PRINT);
//     ASSERT_EQ(tokens[10].type, TokenType::RETURN);
//     ASSERT_EQ(tokens[11].type, TokenType::SUPER);
//     ASSERT_EQ(tokens[12].type, TokenType::THIS);
//     ASSERT_EQ(tokens[13].type, TokenType::TRUE);
//     ASSERT_EQ(tokens[14].type, TokenType::VAR);
//     ASSERT_EQ(tokens[15].type, TokenType::WHILE);
//     ASSERT_EQ(tokens[16].type, TokenType::INT_TYPE);
//     ASSERT_EQ(tokens[17].type, TokenType::FLOAT_TYPE);
//     ASSERT_EQ(tokens[18].type, TokenType::STR_TYPE);
//     ASSERT_EQ(tokens[19].type, TokenType::BOOL_TYPE);
//     ASSERT_EQ(tokens[20].type, TokenType::LIST_TYPE);
//     ASSERT_EQ(tokens[21].type, TokenType::DICT_TYPE);
//     ASSERT_EQ(tokens[22].type, TokenType::ARRAY_TYPE);
//     ASSERT_EQ(tokens[23].type, TokenType::ENUM_TYPE);
//     ASSERT_EQ(tokens[24].type, TokenType::ATTEMPT);
//     ASSERT_EQ(tokens[25].type, TokenType::HANDLE);
//     ASSERT_EQ(tokens[26].type, TokenType::PARALLEL);
//     ASSERT_EQ(tokens[27].type, TokenType::CONCURRENT);
//     ASSERT_EQ(tokens[28].type, TokenType::ASYNC);
//     ASSERT_EQ(tokens[29].type, TokenType::AWAIT);
//     ASSERT_EQ(tokens[30].type, TokenType::IMPORT);
// }

// TEST(ScannerTest, Comments) {
//     std::string source = "// This is a comment\n"
//                         "var x = 10; // Another comment";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 4);

//     ASSERT_EQ(tokens[0].type, TokenType::VAR);
//     ASSERT_EQ(tokens[1].type, TokenType::IDENTIFIER);
//     ASSERT_EQ(tokens[1].lexeme, "x");
//     ASSERT_EQ(tokens[2].type, TokenType::EQUAL);
//     ASSERT_EQ(tokens[3].type, TokenType::NUMBER);
//     ASSERT_EQ(tokens[3].lexeme, "10");
// }

// TEST(ScannerTest, UserType) {
//     std::string source = ":MyType";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::USER_TYPE);
//     ASSERT_EQ(tokens[0].lexeme, "MyType");
// }

// TEST(ScannerTest, Arrow) {
//     std::string source = "->";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::ARROW);
//     ASSERT_EQ(tokens[0].lexeme, "->");
// }

// TEST(ScannerTest, Elif) {
//     std::string source = "elif";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::ELIF);
//     ASSERT_EQ(tokens[0].lexeme, "elif");
// }

// TEST(ScannerTest, Default) {
//     std::string source = "default";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::DEFAULT);
//     ASSERT_EQ(tokens[0].lexeme, "default");
// }

// TEST(ScannerTest, Brackets) {
//     std::string source = "[]";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 2);
//     ASSERT_EQ(tokens[0].type, TokenType::LEFT_BRACKET);
//     ASSERT_EQ(tokens[0].lexeme, "[");
//     ASSERT_EQ(tokens[1].type, TokenType::RIGHT_BRACKET);
//     ASSERT_EQ(tokens[1].lexeme, "]");
// }

// TEST(ScannerTest, Match) {
//     std::string source = "match";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::MATCH);
//     ASSERT_EQ(tokens[0].lexeme, "match");
// }

// TEST(ScannerTest, In) {
//     std::string source = "in";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::IN);
//     ASSERT_EQ(tokens[0].lexeme, "in");
// }

// TEST(ScannerTest, Enum) {
//     std::string source = "enum";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::ENUM);
//     ASSERT_EQ(tokens[0].lexeme, "enum");
// }

// TEST(ScannerTest, UnterminatedString) {
//     std::string source = "\"Hello, world!";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::EOF_TOKEN);
// }

// TEST(ScannerTest, UnterminatedString2) {
//     std::string source = "'single quote";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::EOF_TOKEN);
// }

// TEST(ScannerTest, InvalidCharacter) {
//     std::string source = "123$";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 1);
//     ASSERT_EQ(tokens[0].type, TokenType::EOF_TOKEN);
// }

// TEST(ScannerTest, LineNumbers) {
//     std::string source = "var x = 10;\n"
//                         "print x;";
//     Scanner scanner(source);
//     std::vector<Token> tokens = scanner.scanTokens();

//     ASSERT_EQ(tokens.size(), 6);
//     ASSERT_EQ(tokens[0].line, 1);
//     ASSERT_EQ(tokens[1].line, 1);
//     ASSERT_EQ(tokens[2].line, 1);
//     ASSERT_EQ(tokens[3].line, 1);
//     ASSERT_EQ(tokens[4].line, 2);
//     ASSERT_EQ(tokens[5].line, 2);
// }

// TEST(ScannerTest, GetToken) {
//     std::string source = "var x = 10;";
//     Scanner scanner(source);
//     scanner.scanTokens();

//     ASSERT_EQ(scanner.getToken().type, TokenType::VAR);
//     ASSERT_EQ(scanner.getToken().lexeme, "var");

//     scanner.advance();
//     ASSERT_EQ(scanner.getToken().type, TokenType::IDENTIFIER);
//     ASSERT_EQ(scanner.getToken().lexeme, "x");

//     scanner.advance();
//     ASSERT_EQ(scanner.getToken().type, TokenType::EQUAL);
//     ASSERT_EQ(scanner.getToken().lexeme, "=");

//     scanner.advance();
//     ASSERT_EQ(scanner.getToken().type, TokenType::NUMBER);
//     ASSERT_EQ(scanner.getToken().lexeme, "10");

//     scanner.advance();
//     ASSERT_EQ(scanner.getToken().type, TokenType::SEMICOLON);
//     ASSERT_EQ(scanner.getToken().lexeme, ";");
// }

// TEST(ScannerTest, GetNextToken) {
//     std::string source = "var x = 10;";
//     Scanner scanner(source);
//     scanner.scanTokens();

//     ASSERT_EQ(scanner.getNextToken().type, TokenType::VAR);
//     ASSERT_EQ(scanner.getNextToken().lexeme, "var");

//     ASSERT_EQ(scanner.getNextToken().type, TokenType::IDENTIFIER);
//     ASSERT_EQ(scanner.getNextToken().lexeme, "x");

//     ASSERT_EQ(scanner.getNextToken().type, TokenType::EQUAL);
//     ASSERT_EQ(scanner.getNextToken().lexeme, "=");

//     ASSERT_EQ(scanner.getNextToken().type, TokenType::NUMBER);
//     ASSERT_EQ(scanner.getNextToken().lexeme, "10");

//     ASSERT_EQ(scanner.getNextToken().type, TokenType::SEMICOLON);
//     ASSERT_EQ(scanner.getNextToken().lexeme, ";");
// }

// TEST(ScannerTest, GetPrevToken) {
//     std::string source = "var x = 10;";
//     Scanner scanner(source);
//     scanner.scanTokens();

//     ASSERT_EQ(scanner.getPrevToken().type, TokenType::SEMICOLON);
//     ASSERT_EQ(scanner.getPrevToken().lexeme, ";");

//     ASSERT_EQ(scanner.getPrevToken().type, TokenType::NUMBER);
//     ASSERT_EQ(scanner.getPrevToken().lexeme, "10");

//     ASSERT_EQ(scanner.getPrevToken().type, TokenType::EQUAL);
//     ASSERT_EQ(scanner.getPrevToken().lexeme, "=");

//     ASSERT_EQ(scanner.getPrevToken().type, TokenType::IDENTIFIER);
//     ASSERT_EQ(scanner.getPrevToken().lexeme, "x");

//     ASSERT_EQ(scanner.getPrevToken().type, TokenType::VAR);
//     ASSERT_EQ(scanner.getPrevToken().lexeme, "var");
// }

// TEST(ScannerTest, ToString) {
//     std::string source = "var x = 10;";
//     Scanner scanner(source);
//     scanner.scanTokens();

//     std::string expected = "Token: var | Type: VAR | Line: 1\n"
//                           "Token: x | Type: INDENTIFIER: x | Line: 1\n"
//                           "Token: = | Type: EQUAL | Line: 1\n"
//                           "Token: 10 | Type: NUMBER: 10 | Line: 1\n"
//                           "Token: ; | Type: SEMICOLON | Line: 1\n"
//                           "Token:  | Type: EOF_TOKEN | Line: 1\n";

//     ASSERT_EQ(scanner.toString(), expected);
// }
