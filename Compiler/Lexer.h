#ifndef LEXER_H
#define LEXER_H

#include "../Common/Token.h"
#include <vector>
#include <string>
#include <map>

class Lexer {
private:
    std::string source;
    size_t pos;
    int line;
    int column;

    std::map<std::string, TokenType> keywords;

    char peek() const;
    char advance();
    void skipWhitespace();
    Token readIdentifier();
    Token readNumber();
    Token readString();

public:
    Lexer(const std::string& src);
    std::vector<Token> tokenize();
};

#endif