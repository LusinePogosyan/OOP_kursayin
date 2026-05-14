#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include <stdexcept>
#include "Lexer.h"
#include "ASTNodes.h"

class Parser {
private:
    std::vector<Token> tokens;
    size_t pos;
    int loopDepth = 0;
    int switchDepth = 0;

    // Helper functions
    Token peek() const;
    Token previous() const;
    Token advance();
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const std::string& message);
    bool isTypeToken(TokenType type) const;

    // Required project split points
    std::unique_ptr<ASTNode> prog_parser();
    std::unique_ptr<ASTNode> func_parser(const std::string& retType, const std::string& name);
    std::vector<std::unique_ptr<ASTNode>> func_code();

    // Parsing methods
    std::unique_ptr<ASTNode> declaration(const std::string& type, const std::string& name, bool isStatic = false, bool isGlobal = false, bool isExtern = false);
    std::unique_ptr<ASTNode> statement();
    std::unique_ptr<ASTNode> switchStatement();
    std::unique_ptr<ASTNode> returnStatement();
    std::unique_ptr<ASTNode> breakStatement();
    std::unique_ptr<ASTNode> continueStatement();
    std::unique_ptr<ASTNode> ifStatement();
    std::unique_ptr<ASTNode> whileStatement();
    std::unique_ptr<ASTNode> doWhileStatement();
    std::unique_ptr<ASTNode> forStatement();
    std::unique_ptr<ASTNode> printStatement();
    std::vector<std::unique_ptr<ASTNode>> block();
    
    std::unique_ptr<ExprNode> expression();
    std::unique_ptr<ExprNode> assignment();
    std::unique_ptr<ExprNode> ternary();
    std::unique_ptr<ExprNode> logicalOr();
    std::unique_ptr<ExprNode> logicalAnd();
    std::unique_ptr<ExprNode> bitwiseOr();
    std::unique_ptr<ExprNode> bitwiseXor();
    std::unique_ptr<ExprNode> bitwiseAnd();
    std::unique_ptr<ExprNode> shift();
    std::unique_ptr<ExprNode> equality();
    std::unique_ptr<ExprNode> comparison();
    std::unique_ptr<ExprNode> term();
    std::unique_ptr<ExprNode> factor();
    std::unique_ptr<ExprNode> unary();
    std::unique_ptr<ExprNode> primary();

public:
    Parser(const std::vector<Token>& t) : tokens(t), pos(0) {}
    std::vector<std::unique_ptr<ASTNode>> parse();
};

#endif