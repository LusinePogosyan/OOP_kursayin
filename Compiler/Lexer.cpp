#include "Lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& src) : source(src), pos(0), line(1), column(1) {
    // Keywords initialization
    keywords["if"] = TokenType::If;
    keywords["else"] = TokenType::Else;
    keywords["while"] = TokenType::While;
    keywords["for"] = TokenType::For;
    keywords["do"] = TokenType::Do;
    keywords["switch"] = TokenType::Switch;
    keywords["case"] = TokenType::Case;
    keywords["default"] = TokenType::Default;
    keywords["int"] = TokenType::Int;
    keywords["float"] = TokenType::Float;
    keywords["double"] = TokenType::Double;
    keywords["char"] = TokenType::Char;
    keywords["void"] = TokenType::Void;
    keywords["return"] = TokenType::Return;
    keywords["break"] = TokenType::Break;
    keywords["continue"] = TokenType::Continue;
    keywords["static"] = TokenType::Static;
    keywords["extern"] = TokenType::Extern;
    keywords["inline"] = TokenType::Inline;
    keywords["true"] = TokenType::True;
    keywords["false"] = TokenType::False;
    keywords["class"] = TokenType::Class;
    keywords["struct"] = TokenType::Struct;
    keywords["union"] = TokenType::Union;
    keywords["enum"] = TokenType::Enum;
    keywords["public"] = TokenType::Public;
    keywords["private"] = TokenType::Private;
    keywords["static_cast"] = TokenType::StaticCast;
    keywords["dynamic_cast"] = TokenType::DynamicCast;
    keywords["goto"] = TokenType::Goto;
    keywords["print"] = TokenType::Print;
}

char Lexer::peek() const {
    if (pos >= source.size()) return '\0';
    return source[pos];
}

char Lexer::advance() {
    char c = source[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (isspace(peek())) {
        advance();
    }
    // Հեռացնում ենք մեկտողանի մեկնաբանությունները (//)
    if (peek() == '/' && source[pos + 1] == '/') {
        while (peek() != '\n' && peek() != '\0') advance();
        skipWhitespace();
    }
}

Token Lexer::readIdentifier() {
    std::string result;
    int startCol = column;
    while (isalnum(peek()) || peek() == '_') {
        result += advance();
    }
    if (keywords.count(result)) {
        return Token(keywords[result], result, line, startCol);
    }
    return Token(TokenType::Identifier, result, line, startCol);
}

Token Lexer::readNumber() {
    std::string result;
    int startCol = column;
    while (isdigit(peek())) {
        result += advance();
    }
    // Աջակցում ենք float/double կետով թվերին
    if (peek() == '.') {
        result += advance();
        while (isdigit(peek())) result += advance();
    }
    return Token(TokenType::Number, result, line, startCol);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < source.size()) {
        skipWhitespace();
        if (pos >= source.size()) break;

        char c = peek();
        int startCol = column;

        if (isalpha(c) || c == '_') {
            tokens.push_back(readIdentifier());
        } else if (isdigit(c)) {
            tokens.push_back(readNumber());
        } else {
            advance();
            switch (c) {
                case '+':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::PlusAssign, "+=", line, startCol)); }
                    else tokens.push_back(Token(TokenType::Plus, "+", line, startCol));
                    break;
                case '-':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::MinusAssign, "-=", line, startCol)); }
                    else tokens.push_back(Token(TokenType::Minus, "-", line, startCol));
                    break;
                case '*':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::StarAssign, "*=", line, startCol)); }
                    else tokens.push_back(Token(TokenType::Star, "*", line, startCol));
                    break;
                case '/':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::SlashAssign, "/=", line, startCol)); }
                    else tokens.push_back(Token(TokenType::Slash, "/", line, startCol));
                    break;
                case '%':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::PercentAssign, "%=", line, startCol)); }
                    else tokens.push_back(Token(TokenType::Percent, "%", line, startCol));
                    break;
                case '=':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::Eq, "==", line, startCol)); }
                    else tokens.push_back(Token(TokenType::Assign, "=", line, startCol));
                    break;
                case '!':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::Neq, "!=", line, startCol)); }
                    else tokens.push_back(Token(TokenType::Bang, "!", line, startCol));
                    break;
                case '<':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::Leq, "<=", line, startCol)); }
                    else if (peek() == '<') {
                        advance();
                        if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::ShiftLeftAssign, "<<=", line, startCol)); }
                        else tokens.push_back(Token(TokenType::ShiftLeft, "<<", line, startCol));
                    }
                    else tokens.push_back(Token(TokenType::Lt, "<", line, startCol));
                    break;
                case '>':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::Geq, ">=", line, startCol)); }
                    else if (peek() == '>') {
                        advance();
                        if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::ShiftRightAssign, ">>=", line, startCol)); }
                        else tokens.push_back(Token(TokenType::ShiftRight, ">>", line, startCol));
                    }
                    else tokens.push_back(Token(TokenType::Gt, ">", line, startCol));
                    break;
                case '&':
                    if (peek() == '&') { advance(); tokens.push_back(Token(TokenType::And, "&&", line, startCol)); }
                    else if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::AndAssign, "&=", line, startCol)); }
                    else tokens.push_back(Token(TokenType::BitAnd, "&", line, startCol));
                    break;
                case '|':
                    if (peek() == '|') { advance(); tokens.push_back(Token(TokenType::Or, "||", line, startCol)); }
                    else if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::OrAssign, "|=", line, startCol)); }
                    else tokens.push_back(Token(TokenType::BitOr, "|", line, startCol));
                    break;
                case '^':
                    if (peek() == '=') { advance(); tokens.push_back(Token(TokenType::XorAssign, "^=", line, startCol)); }
                    else tokens.push_back(Token(TokenType::BitXor, "^", line, startCol));
                    break;
                case '~': tokens.push_back(Token(TokenType::BitNot, "~", line, startCol)); break;
                case '(': tokens.push_back(Token(TokenType::LParen, "(", line, startCol)); break;
                case ')': tokens.push_back(Token(TokenType::RParen, ")", line, startCol)); break;
                case '{': tokens.push_back(Token(TokenType::LBrace, "{", line, startCol)); break;
                case '}': tokens.push_back(Token(TokenType::RBrace, "}", line, startCol)); break;
                case '[': tokens.push_back(Token(TokenType::LBracket, "[", line, startCol)); break;
                case ']': tokens.push_back(Token(TokenType::RBracket, "]", line, startCol)); break;
                case ';': tokens.push_back(Token(TokenType::Semicolon, ";", line, startCol)); break;
                case ',': tokens.push_back(Token(TokenType::Comma, ",", line, startCol)); break;
                case '?': tokens.push_back(Token(TokenType::Question, "?", line, startCol)); break;
                case ':':
                    if (peek() == ':') { advance(); tokens.push_back(Token(TokenType::DoubleColon, "::", line, startCol)); }
                    else tokens.push_back(Token(TokenType::Colon, ":", line, startCol));
                    break;
                default:
                    throw std::runtime_error("Unknown character at line " + std::to_string(line));
            }
        }
    }
    tokens.push_back(Token(TokenType::EndOfFile, "", line, column));
    return tokens;
}