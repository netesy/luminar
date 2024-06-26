#pragma once

#include <string>

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
    MUT,        // mut
    THIS,       // this
    TRUE,       // true
    ELIF,       //elif
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

struct Token
{
    TokenType type;
    std::string lexeme;
    std::string filename;
    std::string filepath;
    int column;
    int line;
};
