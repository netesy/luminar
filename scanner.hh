// scanner.hh
#pragma once

#include <iostream>
#include <string>
#include <variant>
#include <vector>

enum class TokenType {
    // Group: Delimiters
    LEFT_PAREN,    // (
    RIGHT_PAREN,   // )
    LEFT_BRACE,    // {
    RIGHT_BRACE,   // }
    LEFT_BRACKET,  // [
    RIGHT_BRACKET, // ]
    COMMA,         // ,
    DOT,           // .
    COLON,         // :
    SEMICOLON,     // ;
    QUESTION,      // ?
    ARROW,         // ->

    // Group: Operators
    PLUS,          // +
    MINUS,         // -
    SLASH,         // /
    MODULUS,       // %
    STAR,          // *
    BANG,          // !
    BANG_EQUAL,    // !=
    EQUAL,         // =
    PLUS_EQUAL,    // +=
    MINUS_EQUAL,   // -=
    EQUAL_EQUAL,   // ==
    GREATER,       // >
    GREATER_EQUAL, // >=
    LESS,          // <
    LESS_EQUAL,    // <=

    // Group: Literals
    STRING,     // string literals
    NUMBER,     // numeric literals
    IDENTIFIER, // variable/function names

    // Group: Types
    INT_TYPE,      // int
    FLOAT_TYPE,    // float
    STR_TYPE,      // str
    BOOL_TYPE,     // bool
    USER_TYPE,     // user-defined types
    LIST_TYPE,     // list
    DICT_TYPE,     // dictionary
    ARRAY_TYPE,    // array
    ENUM_TYPE,     // enum
    FUNCTION_TYPE, // function

    // Group: Keywords
    FN,         // fn
    IF,         // if
    IN,         // in
    OR,         // or
    NIL,        // nil
    AND,        // and
    FOR,        // for
    VAR,        // var
    THIS,       // this
    TRUE,       // true
    ELSE,       // else
    ENUM,       // enum
    ASYNC,      // async
    AWAIT,      // await
    CLASS,      // class
    WHILE,      // while
    MATCH,      // match
    FALSE,      // false
    PRINT,      // print
    SUPER,      // super
    IMPORT,     // import
    RETURN,     // return
    HANDLE,     // handle
    DEFAULT,    // default
    ATTEMPT,    // attempt
    PARALLEL,   // parallel
    CONCURRENT, // concurrent

    // Other
    UNDEFINED, // undefined token
    EOF_TOKEN  // end of file token
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};

class Scanner {
public:
    explicit Scanner(const std::string &source)
        : current(0)
        , start(0)
        , line(1)
        , source(source)
    {
        //        tokens = scanTokens();
    }

    std::vector<Token> scanTokens();
    void scanToken();
    bool isAtEnd() const;

    std::string toString() const;
    char advance();
    char peek() const;
    char peekNext() const;
    char peekPrevious() const;
    std::string tokenTypeToString(TokenType type) const;
    int getLine() const { return line; }
    int getCurrent() const { return current; }
    std::string getLexeme() const { return source.substr(start, current - start); }
    size_t current;
    size_t start;
    int line;

    const Token getToken();
    const Token getNextToken();
    const Token getPrevToken();

    std::vector<Token> tokens;

private:
    const std::string& source;
    Token currentToken;

    void addToken(TokenType type);
    void addToken(TokenType type, const std::string& text);
    bool match(char expected);
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    void string();
    void number();
    void identifier();
    void error(const std::string& message);
    TokenType checkKeyword(size_t begin,
                           size_t length,
                           const std::string &rest,
                           TokenType type) const;
    Token getTokenFromChar(char c);
};
