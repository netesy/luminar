// #include "gtest/gtest.h"
// #include "parser.hh"
// #include "scanner.hh"

// TEST(ParserTest, EmptyProgram) {
//   Scanner scanner("");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 1);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::HALT);
// }

// TEST(ParserTest, NumberLiteral) {
//   Scanner scanner("123;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 2);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 123.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::HALT);
// }

// TEST(ParserTest, StringLiteral) {
//   Scanner scanner("\"Hello, world!\";");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 2);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_STR);
//   ASSERT_EQ(std::get<std::string>(bytecode[0].value), "Hello, world!");
//   ASSERT_EQ(bytecode[1].opcode, Opcode::HALT);
// }

// TEST(ParserTest, BooleanLiteral) {
//   Scanner scanner("true;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 2);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<bool>(bytecode[0].value), true);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleAddition) {
//   Scanner scanner("1 + 2;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 4);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 1.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[1].value), 2.0);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::ADD);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleSubtraction) {
//   Scanner scanner("1 - 2;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 4);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 1.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[1].value), 2.0);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::SUBTRACT);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleMultiplication) {
//   Scanner scanner("1 * 2;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 4);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 1.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[1].value), 2.0);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::MULTIPLY);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleDivision) {
//   Scanner scanner("1 / 2;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 4);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 1.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[1].value), 2.0);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::DIVIDE);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleModulus) {
//   Scanner scanner("1 % 2;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 4);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 1.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[1].value), 2.0);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::MODULUS);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleNegation) {
//   Scanner scanner("-1;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 3);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 1.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::NEGATE);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleNot) {
//   Scanner scanner("!true;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 3);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<bool>(bytecode[0].value), true);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::NOT);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimplePrintStatement) {
//   Scanner scanner("print(123);");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 3);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 123.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::PRINT);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleVariableDeclaration) {
//   Scanner scanner("var x = 123;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 4);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 123.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::STORE_VARIABLE);
//   ASSERT_EQ(std::get<int32_t>(bytecode[1].value), 0);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::DECLARE_VARIABLE);
//   ASSERT_EQ(std::get<int32_t>(bytecode[2].value), 0);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleVariableAssignment) {
//   Scanner scanner("x = 123;");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 3);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 123.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::STORE_VARIABLE);
//   ASSERT_EQ(std::get<int32_t>(bytecode[1].value), 0);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleIfStatement) {
//   Scanner scanner("if (true) { print(123); }");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 7);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<bool>(bytecode[0].value), true);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::JUMP_IF_FALSE);
//   ASSERT_EQ(std::get<int32_t>(bytecode[1].value), 4);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[2].value), 123.0);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::PRINT);
//   ASSERT_EQ(bytecode[4].opcode, Opcode::JUMP);
//   ASSERT_EQ(std::get<int32_t>(bytecode[4].value), 1);
//   ASSERT_EQ(bytecode[5].opcode, Opcode::HALT);
//   ASSERT_EQ(bytecode[6].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleWhileLoop) {
//   Scanner scanner("while (true) { print 123; }");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 8);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<bool>(bytecode[0].value), true);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::JUMP_IF_FALSE);
//   ASSERT_EQ(std::get<int32_t>(bytecode[1].value), 5);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[2].value), 123.0);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::PRINT);
//   ASSERT_EQ(bytecode[4].opcode, Opcode::JUMP);
//   ASSERT_EQ(std::get<int32_t>(bytecode[4].value), -4);
//   ASSERT_EQ(bytecode[5].opcode, Opcode::HALT);
//   ASSERT_EQ(bytecode[6].opcode, Opcode::HALT);
//   ASSERT_EQ(bytecode[7].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleFunctionDeclaration) {
//   Scanner scanner("fn myFunction() { print 123; }");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 4);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::DEFINE_FUNCTION);
//   ASSERT_EQ(std::get<std::string>(bytecode[0].value), "myFunction");
//   ASSERT_EQ(bytecode[1].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[1].value), 123.0);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::PRINT);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleFunctionCall) {
//   Scanner scanner("myFunction();");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 2);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::INVOKE_FUNCTION);
//   ASSERT_EQ(std::get<std::string>(bytecode[0].value), "myFunction");
//   ASSERT_EQ(bytecode[1].opcode, Opcode::HALT);
// }

// TEST(ParserTest, FunctionCallWithArguments) {
//   Scanner scanner("myFunction(1, 2, \"hello\");");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 6);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 1.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::PUSH_ARGS);
//   ASSERT_EQ(std::get<std::string>(bytecode[1].value), "1");
//   ASSERT_EQ(bytecode[2].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[2].value), 2.0);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::PUSH_ARGS);
//   ASSERT_EQ(std::get<std::string>(bytecode[3].value), "2");
//   ASSERT_EQ(bytecode[4].opcode, Opcode::LOAD_STR);
//   ASSERT_EQ(std::get<std::string>(bytecode[4].value), "hello");
//   ASSERT_EQ(bytecode[5].opcode, Opcode::INVOKE_FUNCTION);
//   ASSERT_EQ(std::get<std::string>(bytecode[5].value), "myFunction");
//   ASSERT_EQ(bytecode[6].opcode, Opcode::HALT);
// }

// TEST(ParserTest, SimpleMatchStatement) {
//   Scanner scanner("match (1) { 1: print 123; }");
//   Parser parser(scanner);
//   Bytecode bytecode = parser.parse();
//   ASSERT_EQ(bytecode.size(), 7);
//   ASSERT_EQ(bytecode[0].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[0].value), 1.0);
//   ASSERT_EQ(bytecode[1].opcode, Opcode::PATTERN_MATCH);
//   ASSERT_EQ(bytecode[2].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[2].value), 1.0);
//   ASSERT_EQ(bytecode[3].opcode, Opcode::PATTERN_MATCH);
//   ASSERT_EQ(bytecode[4].opcode, Opcode::LOAD_CONST);
//   ASSERT_EQ(std::get<double>(bytecode[4].value), 123.0);
//   ASSERT_EQ(bytecode[5].opcode, Opcode::PRINT);
//   ASSERT_EQ(bytecode[6].opcode, Opcode::HALT);
// }

// int main(int argc, char **argv) {
//   ::testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }
