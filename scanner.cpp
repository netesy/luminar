// scanner.cpp
#include "scanner.hh"
#include "debugger.hh"
#include <chrono>
#include <string>

std::vector<Token> Scanner::scanTokens()
{
    auto start_time = std::chrono::high_resolution_clock::now();
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }

    tokens.push_back(
        {TokenType::EOF_TOKEN, "", filename, filepath, static_cast<int>(current), line});
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << "Scanning completed in " << duration.count() << " microseconds." << std::endl;
    return tokens;
}

void Scanner::scanToken()
{
    char c = advance();
    switch (c) {
    case '(':
        addToken(TokenType::LEFT_PAREN);
        break;
    case ')':
        addToken(TokenType::RIGHT_PAREN);
        break;
    case '{':
        addToken(TokenType::LEFT_BRACE);
        break;
    case '}':
        addToken(TokenType::RIGHT_BRACE);
        break;
    case '[':
        addToken(TokenType::LEFT_BRACKET);
        break;
    case ']':
        addToken(TokenType::RIGHT_BRACKET);
        break;
    case ',':
        addToken(TokenType::COMMA);
        break;
    case '.':
        addToken(TokenType::DOT);
        break;
    case '-':
        if (match('=')) {
            addToken(TokenType::MINUS_EQUAL); // Handle minus-equal first
        } else if (match('>')) {
            addToken(TokenType::ARROW);
        } else {
            addToken(TokenType::MINUS); // Default minus
        }
        break;
    case '+':
        addToken(match('=') ? TokenType::PLUS_EQUAL : TokenType::PLUS);
        break;
    case '?':
        addToken(TokenType::QUESTION);
        break;
    case ':':
        addToken(TokenType::COLON);
        break;
    case ';':
        addToken(TokenType::SEMICOLON);
        break;
    case '*':
        addToken(TokenType::STAR);
        break;
    case '!':
        addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        break;
    case '=':
        addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        break;
    case '<':
        addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        break;
    case '>':
        addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        break;
    case '_':
        addToken(TokenType::DEFAULT);
        break;
    case '/':
        if (match('/')) {
            // A comment goes until the end of the line.
            while (peek() != '\n' && !isAtEnd())
                advance();
        } else {
            addToken(TokenType::SLASH);
        }
        break;
    case '%':
        addToken(TokenType::MODULUS);
        break;
    case ' ':
    case '\r':
    case '\t':
        // Ignore whitespace.
        break;
    case '\n':
        line++;
        break;
    case '"':
    case '\'':
        string();
        break;
    default:
        if (isDigit(c)) {
            number();
        } else if (isAlpha(c)) {
            identifier();
        } else {
            error("Invalid character.");
        }
        break;
    }
}

Token Scanner::getTokenFromChar(char c)
{
    switch (c) {
    case '(':
        addToken(TokenType::LEFT_PAREN);
        break;
    case ')':
        addToken(TokenType::RIGHT_PAREN);
        break;
    case '{':
        addToken(TokenType::LEFT_BRACE);
        break;
    case '}':
        addToken(TokenType::RIGHT_BRACE);
        break;
    case '[':
        addToken(TokenType::LEFT_BRACKET);
        break;
    case ']':
        addToken(TokenType::RIGHT_BRACKET);
        break;
    case ',':
        addToken(TokenType::COMMA);
        break;
    case '.':
        addToken(TokenType::DOT);
        break;
    case '-':
        if (match('>')) {
            addToken(TokenType::ARROW);
        } else if (match('=')) {
            addToken(TokenType::MINUS_EQUAL);
        } else {
            addToken(TokenType::MINUS);
        }
        break;
    case '+':
        addToken(match('=') ? TokenType::PLUS_EQUAL : TokenType::PLUS);
        break;
    case '?':
        addToken(TokenType::QUESTION);
        break;
    case ':':
        addToken(TokenType::COLON);
        break;
    case ';':
        addToken(TokenType::SEMICOLON);
        break;
    case '*':
        addToken(TokenType::STAR);
        break;
    case '!':
        addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        break;
    case '=':
        addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        break;
    case '<':
        addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        break;
    case '>':
        addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        break;
    case '_':
        addToken(TokenType::DEFAULT);
        break;
    case '/':
        if (match('/')) {
            // Handle comments if needed
            while (peek() != '\n' && !isAtEnd())
                advance();
        } else {
            addToken(TokenType::SLASH);
        }
        break;
    case '%':
        addToken(TokenType::MODULUS);
        break;
    case ' ':
    case '\r':
    case '\t':
        // Ignore whitespace.
        break;
    case '\n':
        line++;
        break;
    case '"':
    case '\'':
        string();
        break;
    default:
        if (isDigit(c)) {
            number();
        } else if (isAlpha(c)) {
            identifier();
        } else {
            addToken(TokenType::UNDEFINED);
            error("Unexpected character.");
        }
        break;
    }

    return currentToken;
}

bool Scanner::isAtEnd() const
{
    return current >= source.size();
}

char Scanner::advance()
{
    return source[current++];
}

void Scanner::addToken(TokenType type)
{
    addToken(type, tokenTypeToString(type, ""));
}

void Scanner::addToken(TokenType type, const std::string &text)
{
    std::string lexeme = source.substr(start, current - start);
    if (text != "") {
        tokens.push_back({type, text, filename, filepath, int(current), line});
    } else {
        tokens.push_back({type, lexeme, filename, filepath, int(current), line});
    }
    currentToken = tokens.back(); // Update currentToken
}

bool Scanner::match(char expected)
{
    if (isAtEnd())
        return false;
    if (source[current] != expected)
        return false;

    current++;
    return true;
}

const Token Scanner::getToken()
{
    return currentToken;
}

const Token Scanner::getNextToken()
{
    // Assuming tokens is a member variable of type std::vector<Token>
    size_t index = tokens.size();
    if (index < tokens.size()) {
        return tokens[index];
    } else {
        // Handle end of tokens
        return Token{TokenType::EOF_TOKEN, "", filename, filepath, static_cast<int>(current), line};
    }
}

const Token Scanner::getPrevToken()
{
    // Assuming tokens is a member variable of type std::vector<Token>
    size_t index = tokens.size();
    if (index > 0) {
        return tokens[index];
    } else {
        // Handle no previous token
        return Token{TokenType::EOF_TOKEN, "", filename, filepath, static_cast<int>(current), line};
    }
}

char Scanner::peek() const
{
    if (isAtEnd())
        return '\0';
    return source[current];
}

char Scanner::peekNext() const
{
    if (current + 1 >= source.size())
        return '\0';
    return source[current + 1];
}

char Scanner::peekPrevious() const
{
    if (current > start) {
        return source[current - 1];
    }
    return '\0'; // Return null character if no previous character exists
}

bool Scanner::isDigit(char c) const
{
    return c >= '0' && c <= '9';
}

bool Scanner::isAlpha(char c) const
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::isAlphaNumeric(char c) const
{
    return isAlpha(c) || isDigit(c);
}

void Scanner::string()
{
    char quoteType = source[start]; // Store the opening quote type
    advance();                      // Consume the opening quote

    while (peek() != quoteType && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
            error("Unterminated string.");
            return;
        }
        advance();
    }

    if (isAtEnd()) {
        error("Unterminated string.");
        return;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
}

void Scanner::number()
{
    while (isDigit(peek()))
        advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the ".".
        advance();

        while (isDigit(peek()))
            advance();
    }
    std::string value = source.substr(start, current + 1 - start - 1);
    addToken(TokenType::NUMBER, value);
}

void Scanner::identifier()
{
    TokenType type = TokenType::IDENTIFIER;
    // Check if the identifier is preceded by certain keywords or ':'
    if (source[start - 1] == ':') {
        type = TokenType::USER_TYPE; // Preceded by ':', so it's a USER_TYPE
    } else if (start >= 4 && source.substr(start - 4, 4) == "var ") {
        type = TokenType::IDENTIFIER; // Preceded by "var ", so it's a VAR
    } else if (start >= 3 && source.substr(start - 3, 3) == "fn ") {
        type = TokenType::FN; // Preceded by "fn ", so it's a FN
    } else if (start >= 6 && source.substr(start - 6, 6) == "class ") {
        type = TokenType::CLASS; // Preceded by "class ", so it's a CLASS
    }                            /* else {
        type = TokenType::UNDEFINED;
        error("Invalid characters");
    }*/

    while (isAlphaNumeric(peek()))
        advance();

    addToken(checkKeyword(start, current - start, source.substr(start, current - start), type));
}

TokenType Scanner::checkKeyword(size_t begin,
                                size_t length,
                                const std::string &rest,
                                TokenType type) const
{
    //    std::string keyword = source.substr(this->start + begin, length);
    //    std::string target(rest);
    //    if (length == target.length() && keyword == target) {
    //        return type;
    //    }
    //    return TokenType::IDENTIFIER;
    //    // Check if the identifier matches a reserved keyword.
    //    //    std::cout << start;
    //    //    std::cout << length;
    if (rest == "and")
        return TokenType::AND;
    if (rest == "default")
        return TokenType::DEFAULT;
    if (rest == "class")
        return TokenType::CLASS;
    if (rest == "else")
        return TokenType::ELSE;
    if (rest == "elif")
        return TokenType::ELIF;
    if (rest == "false")
        return TokenType::FALSE;
    if (rest == "for")
        return TokenType::FOR;
    if (rest == "fn")
        return TokenType::FN;
    if (rest == "if")
        return TokenType::IF;
    if (rest == "in")
        return TokenType::IN;
    if (rest == "or")
        return TokenType::OR;
    if (rest == "print")
        return TokenType::PRINT;
    if (rest == "return")
        return TokenType::RETURN;
    if (rest == "super")
        return TokenType::SUPER;
    if (rest == "this")
        return TokenType::THIS;
    if (rest == "true")
        return TokenType::TRUE;
    if (rest == "var")
        return TokenType::VAR;
    if (rest == "while")
        return TokenType::WHILE;
    if (rest == "attempt")
        return TokenType::ATTEMPT;
    if (rest == "handle")
        return TokenType::HANDLE;
    if (rest == "parallel")
        return TokenType::PARALLEL;
    if (rest == "concurrent")
        return TokenType::CONCURRENT;
    if (rest == "async")
        return TokenType::ASYNC;
    if (rest == "await")
        return TokenType::AWAIT;
    if (rest == "import")
        return TokenType::IMPORT;
    // Check if the identifier matches a type keyword
    if (rest == "int")
        return TokenType::INT_TYPE;
    if (rest == "i8")
        return TokenType::INT8_TYPE;
    if (rest == "i16")
        return TokenType::INT16_TYPE;
    if (rest == "i32")
        return TokenType::INT32_TYPE;
    if (rest == "i64")
        return TokenType::INT64_TYPE;
    if (rest == "uint")
        return TokenType::UINT_TYPE;
    if (rest == "u8")
        return TokenType::UINT8_TYPE;
    if (rest == "u16")
        return TokenType::UINT16_TYPE;
    if (rest == "u32")
        return TokenType::UINT32_TYPE;
    if (rest == "u64")
        return TokenType::UINT64_TYPE;
    if (rest == "any")
        return TokenType::ANY_TYPE;
    if (rest == "nil")
        return TokenType::NIL_TYPE;
    if (rest == "float")
        return TokenType::FLOAT_TYPE;
    if (rest == "f32")
        return TokenType::FLOAT32_TYPE;
    if (rest == "f64")
        return TokenType::FLOAT64_TYPE;
    if (rest == "str")
        return TokenType::STR_TYPE;
    if (rest == "bool")
        return TokenType::BOOL_TYPE;
    if (rest == "list")
        return TokenType::LIST_TYPE;
    if (rest == "array")
        return TokenType::ARRAY_TYPE;
    if (rest == "dict")
        return TokenType::DICT_TYPE;
    if (rest == "enum")
        return TokenType::ENUM_TYPE;
    if (rest == "sum")
        return TokenType::SUM_TYPE;
    if (rest == "union")
        return TokenType::UNION_TYPE;

    return type;
}

std::string Scanner::toString() const
{
    std::string result;
    for (const auto &token : tokens) {
        result += "Token: " + token.lexeme + " | Type: " + tokenTypeToString(token.type, token.lexeme)
                  + " | Line: " + std::to_string(token.line) + "\n";
    }
    return result;
}

std::string Scanner::tokenTypeToString(TokenType type, std::string value) const
{
    switch (type) {
    case TokenType::LEFT_PAREN:
        return "LEFT_PAREN";
    case TokenType::RIGHT_PAREN:
        return "RIGHT_PAREN";
    case TokenType::LEFT_BRACE:
        return "LEFT_BRACE";
    case TokenType::RIGHT_BRACE:
        return "RIGHT_BRACE";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::DOT:
        return "DOT";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::QUESTION:
        return "QUESTION";
    case TokenType::COLON:
        return "COLON";
    case TokenType::SEMICOLON:
        return "SEMICOLON";
    case TokenType::STAR:
        return "STAR";
    case TokenType::BANG:
        return "BANG";
    case TokenType::BANG_EQUAL:
        return "BANG_EQUAL";
    case TokenType::EQUAL:
        return "EQUAL";
    case TokenType::EQUAL_EQUAL:
        return "EQUAL_EQUAL";
    case TokenType::LESS:
        return "LESS";
    case TokenType::LESS_EQUAL:
        return "LESS_EQUAL";
    case TokenType::GREATER:
        return "GREATER";
    case TokenType::GREATER_EQUAL:
        return "GREATER_EQUAL";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::STRING:
        return "STRING: " + value;
    case TokenType::NUMBER:
        return "NUMBER: " + value;
    case TokenType::IDENTIFIER:
        return "INDENTIFIER: " + value;
    case TokenType::AND:
        return "AND";
    case TokenType::CLASS:
        return "CLASS";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::FALSE:
        return "FALSE";
    case TokenType::FOR:
        return "FOR";
    case TokenType::FN:
        return "FN";
    case TokenType::IF:
        return "IF";
    case TokenType::NIL_TYPE:
        return "NIL";
    case TokenType::OR:
        return "OR";
    case TokenType::PRINT:
        return "PRINT";
    case TokenType::RETURN:
        return "RETURN";
    case TokenType::SUPER:
        return "SUPER";
    case TokenType::THIS:
        return "THIS";
    case TokenType::TRUE:
        return "TRUE";
    case TokenType::VAR:
        return "VAR";
    case TokenType::WHILE:
        return "WHILE";
    case TokenType::INT_TYPE:
        return "INT_TYPE";
    case TokenType::FLOAT_TYPE:
        return "FLOAT_TYPE";
    case TokenType::STR_TYPE:
        return "STR_TYPE";
    case TokenType::BOOL_TYPE:
        return "BOOL_TYPE";
    case TokenType::EOF_TOKEN:
        return "EOF_TOKEN";
    case TokenType::USER_TYPE:
        return "USER_TYPE";
    case TokenType::FUNCTION_TYPE:
        return "FN_TYPE";
    case TokenType::LIST_TYPE:
        return "LIST_TYPE";
    case TokenType::DICT_TYPE:
        return "DICT_TYPE";
    case TokenType::ARRAY_TYPE:
        return "ARRAY_TYPE";
    case TokenType::ENUM_TYPE:
        return "ENUM_TYPE";
    case TokenType::MODULUS:
        return "MODULUS";
    case TokenType::ATTEMPT:
        return "ATTEMPT";
    case TokenType::HANDLE:
        return "HANDLE";
    case TokenType::PARALLEL:
        return "PARELLEL";
    case TokenType::CONCURRENT:
        return "CONCURRENT";
    case TokenType::ASYNC:
        return "ASYNC ";
    case TokenType::AWAIT:
        return "AWAIT";
    case TokenType::IMPORT:
        return "IMPORT";
    case TokenType::ARROW:
        return "ARROW";
    case TokenType::LEFT_BRACKET:
        return "LEFT_BRACKET";
    case TokenType::RIGHT_BRACKET:
        return "RIGHT_BRACKET";
    case TokenType::MATCH:
        return "MATCH";
    case TokenType::IN:
        return "IN";
    case TokenType::DEFAULT:
        return "DEFAULT";
    case TokenType::UNDEFINED:
        return "UNDEFINED";
    case TokenType::ENUM:
        return "ENUM";
    case TokenType::PLUS_EQUAL:
        return "PLUS_EQUAL";
    case TokenType::MINUS_EQUAL:
        return "MINUS_EQUAL";
    case TokenType::ELIF:
        return "ELIF";
    case TokenType::MUT:
        return "MUT";
        break;
    }
    return "UNKNOWN";
}

std::string Scanner::getSource()
{
    return this->source;
}

void Scanner::error(const std::string &message)
{
    Debugger::error(message, currentToken, InterpretationStage::SCANNING, getSource(), "");
}
