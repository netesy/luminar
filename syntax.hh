//// syntax.hpp
//#pragma once

//#include "opcodes.hh"
//#include "scanner.hh"

//class Syntax
//{
//public:
//        void parseFunctionDeclaration(Scanner &scanner);
//        //    void parseForRange(Scanner &scanner);
//        void parseForLoop(Scanner &scanner);
//        void parseWhileLoop(Scanner &scanner);
//        void parseConditional(Scanner &scanner);
//        void parseClassDeclaration(Scanner &scanner);
//        void parseVariableDeclaration(Scanner &scanner);
//        void parseAssignment(Scanner &scanner);
//        void parseExpression(Scanner &scanner);
//        void parseAttempt(Scanner &scanner);
//        void parseString(Scanner &scanner);
//        void parseConcurrent(Scanner &scanner);
//        void parseParallel(Scanner &scanner);
//        void parseAwait(Scanner &scanner);
//        void parseAsync(Scanner &scanner);
//        //    void parseBlock(Scanner &scanner);
//        void ternary(Scanner &scanner);
//        void logicalOr(Scanner &scanner);
//        void logicalAnd(Scanner &scanner);
//        void equality(Scanner &scanner);
//        void comparison(Scanner &scanner);
//        void addition(Scanner &scanner);
//        void subtraction(Scanner &scanner);
//        void multiplication(Scanner &scanner);
//        void division(Scanner &scanner);
//        void modulus(Scanner &scanner);
//        void unary(Scanner &scanner);
//        void primary(Scanner &scanner);
//        void parseIdentifier(Scanner &scanner);
//        void parseType(Scanner &scanner);
//        void parseArguments(Scanner &scanner);
//        void parsePatternMatching(Scanner &scanner);
//        void parseMatchCase(Scanner &scanner);

//        void parsePrintStatement(Scanner &scanner);

//        void parseReturnStatement(Scanner &scanner);

//private:
//        void emit(Opcode op, uint32_t lineNumber, int32_t intValue);
//        void emit(Opcode op, uint32_t lineNumber, float floatValue);
//        void emit(Opcode op, uint32_t lineNumber, bool boolValue);
//        void emit(Opcode op, uint32_t lineNumber, const std::string &stringValue);
//        void emit(Opcode op, uint32_t lineNumber);

//        char advance(Scanner &scanner);
//        bool match(Scanner &scanner, TokenType expected);
//        void error(const std::string &message, int line = 0, int start = 0);
//};
