// scanner.hh
#pragma once

#include "token.hh"
#include <iostream>
#include <string>
#include <variant>
#include <vector>

class Scanner
{
public:
    explicit Scanner(const std::string &source,
                     const std::string &filename = "",
                     const std::string &filepath = "")
        : current(0)
        , start(0)
        , line(1)
        , source(source)
        , filename(filename)
        , filepath(filepath)
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
    std::string tokenTypeToString(TokenType type, std::string value) const;
    int getLine() const { return line; }
    int getCurrent() const { return current; }
    std::string getSource();
    std::string getLexeme() const { return source.substr(start, current - start); }
    size_t current;
    size_t start;
    int line;

    const Token getToken();
    const Token getNextToken();
    const Token getPrevToken();
    std::string getFilename() const { return filename; }
    std::string getFilepath() const { return filepath; }

    std::vector<Token> tokens;

private:
    const std::string &source;
    std::string filename;
    std::string filepath;
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
    TokenType checkKeyword(const std::string &identifier) const;
    Token getTokenFromChar(char c);
};
