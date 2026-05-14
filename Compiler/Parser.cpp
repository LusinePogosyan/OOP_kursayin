#include "Parser.h"

Token Parser::peek() const {
    return tokens[pos];
}

Token Parser::previous() const {
    return tokens[pos - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) {
        pos++;
    }
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EndOfFile;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return false;
    }
    return peek().type == type;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    throw std::runtime_error(message);
}

bool Parser::isTypeToken(TokenType type) const {
    return type == TokenType::Int || type == TokenType::Void ||
           type == TokenType::Float || type == TokenType::Double ||
           type == TokenType::Char || type == TokenType::Identifier;
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> nodes;
    while (!isAtEnd()) {
        nodes.push_back(prog_parser());
    }
    return nodes;
}

std::unique_ptr<ASTNode> Parser::prog_parser() {
    if (match({TokenType::Class, TokenType::Struct, TokenType::Union})) {
        if (previous().type == TokenType::Union) {
            auto un = std::make_unique<UnionNode>();
            un->name = consume(TokenType::Identifier, "Expected union name").value;
            consume(TokenType::LBrace, "Expected {");
            while (!check(TokenType::RBrace) && !isAtEnd()) {
                advance();
            }
            consume(TokenType::RBrace, "Expected }");
            if (check(TokenType::Semicolon)) {
                advance();
            }
            return un;
        }
        auto node = std::make_unique<ClassNode>();
        node->isStruct = (previous().type == TokenType::Struct);
        node->name = consume(TokenType::Identifier, "Expected class/struct name").value;
        consume(TokenType::LBrace, "Expected {");
        while (!check(TokenType::RBrace) && !isAtEnd()) {
            advance();
        }
        consume(TokenType::RBrace, "Expected }");
        if (check(TokenType::Semicolon)) {
            advance();
        }
        return node;
    }

    bool isInline = match({TokenType::Inline});
    (void)isInline;
    bool isStatic = match({TokenType::Static});
    bool isExtern = match({TokenType::Extern});
    if (!isTypeToken(peek().type)) {
        throw std::runtime_error("Top-level declaration must start with a type");
    }
    std::string type = advance().value;
    std::string name = consume(TokenType::Identifier, "Expected identifier").value;

    if (check(TokenType::LParen)) {
        return func_parser(type, name);
    }
    return declaration(type, name, isStatic && !isExtern, true, isExtern);
}

std::unique_ptr<ASTNode> Parser::func_parser(const std::string& retType, const std::string& name) {
    auto func = std::make_unique<FunctionNode>();
    func->returnType = retType;
    func->name = name;

    consume(TokenType::LParen, "Expected (");
    while (!check(TokenType::RParen) && !isAtEnd()) {
        if (!isTypeToken(peek().type)) {
            throw std::runtime_error("Expected parameter type");
        }
        std::string pType = advance().value;
        if (match({TokenType::BitAnd})) {
            pType += "&";
        } else if (match({TokenType::Star})) {
            pType += "*";
        }
        std::string pName = consume(TokenType::Identifier, "Expected parameter name").value;
        func->params.push_back({pType, pName});
        if (check(TokenType::Comma)) {
            advance();
        } else {
            break;
        }
    }
    consume(TokenType::RParen, "Expected )");
    if (check(TokenType::Semicolon)) {
        advance();
        return func;
    }
    func->body = func_code();
    return func;
}

std::vector<std::unique_ptr<ASTNode>> Parser::func_code() {
    consume(TokenType::LBrace, "Expected {");
    std::vector<std::unique_ptr<ASTNode>> stmts;
    while (!check(TokenType::RBrace) && !isAtEnd()) {
        stmts.push_back(statement());
    }
    consume(TokenType::RBrace, "Expected }");
    return stmts;
}

std::unique_ptr<ASTNode> Parser::declaration(const std::string& type, const std::string& name, bool isStatic, bool isGlobal, bool isExtern) {
    std::unique_ptr<ExprNode> init = nullptr;
    if (match({TokenType::Assign})) {
        init = expression();
    }
    consume(TokenType::Semicolon, "Expected ; after declaration");
    return std::make_unique<DeclarationNode>(type, name, isStatic, isGlobal, isExtern, std::move(init));
}

std::unique_ptr<ASTNode> Parser::statement() {
    if (check(TokenType::If)) {
        advance();
        return ifStatement();
    }
    if (check(TokenType::While)) {
        advance();
        return whileStatement();
    }
    if (check(TokenType::Do)) {
        advance();
        return doWhileStatement();
    }
    if (check(TokenType::For)) {
        advance();
        return forStatement();
    }
    if (check(TokenType::Switch)) {
        advance();
        return switchStatement();
    }
    if (check(TokenType::Return)) {
        advance();
        return returnStatement();
    }
    if (check(TokenType::Break)) {
        advance();
        return breakStatement();
    }
    if (check(TokenType::Continue)) {
        advance();
        return continueStatement();
    }
    if (check(TokenType::Print)) {
        advance();
        return printStatement();
    }
    if (check(TokenType::Static)) {
        advance();
        std::string type = consume(TokenType::Int, "Expected type after static").value;
        std::string name = consume(TokenType::Identifier, "Expected name").value;
        return declaration(type, name, true, false, false);
    }
    if (check(TokenType::Inline)) {
        advance();
    }
    if (check(TokenType::Extern)) {
        advance();
        std::string type = consume(TokenType::Int, "Expected type after extern").value;
        std::string name = consume(TokenType::Identifier, "Expected identifier").value;
        return declaration(type, name, false, false, true);
    }
    if (check(TokenType::Int) || check(TokenType::Void) || check(TokenType::Float) ||
        check(TokenType::Double) || check(TokenType::Char)) {
        std::string type = advance().value;
        std::string name = consume(TokenType::Identifier, "Expected identifier").value;
        return declaration(type, name, false, false, false);
    }

    auto expr = expression();
    consume(TokenType::Semicolon, "Expected ; after expression");
    return expr;
}

std::unique_ptr<ASTNode> Parser::switchStatement() {
    auto sw = std::make_unique<SwitchNode>();
    consume(TokenType::LParen, "Expected ( after switch");
    sw->condition = expression();
    consume(TokenType::RParen, "Expected )");
    consume(TokenType::LBrace, "Expected {");

    switchDepth++;
    SwitchNode::Section* currentSection = nullptr;
    while (!check(TokenType::RBrace) && !isAtEnd()) {
        if (match({TokenType::Case})) {
            int caseValue = std::stoi(consume(TokenType::Number, "Expected case integer").value);
            consume(TokenType::Colon, "Expected :");
            sw->orderedSections.push_back({false, caseValue, {}});
            currentSection = &sw->orderedSections.back();
            continue;
        }
        if (match({TokenType::Default})) {
            consume(TokenType::Colon, "Expected :");
            sw->orderedSections.push_back({true, 0, {}});
            currentSection = &sw->orderedSections.back();
            continue;
        }
        if (currentSection == nullptr) {
            throw std::runtime_error("Switch statement requires case/default label before statements");
        }
        currentSection->statements.push_back(statement());
    }
    switchDepth--;

    consume(TokenType::RBrace, "Expected } after switch body");
    return sw;
}

std::unique_ptr<ASTNode> Parser::returnStatement() {
    std::unique_ptr<ExprNode> expr = nullptr;
    if (!check(TokenType::Semicolon)) {
        expr = expression();
    }
    consume(TokenType::Semicolon, "Expected ; after return");
    return std::make_unique<ReturnNode>(std::move(expr));
}

std::unique_ptr<ASTNode> Parser::breakStatement() {
    if (switchDepth == 0 && loopDepth == 0) {
        throw std::runtime_error("Invalid 'break': only allowed inside switch or loop");
    }
    consume(TokenType::Semicolon, "Expected ; after break");
    return std::make_unique<BreakNode>();
}

std::unique_ptr<ASTNode> Parser::continueStatement() {
    if (loopDepth == 0) {
        throw std::runtime_error("Invalid 'continue': only allowed inside loop");
    }
    consume(TokenType::Semicolon, "Expected ; after continue");
    return std::make_unique<ContinueNode>();
}

std::unique_ptr<ASTNode> Parser::ifStatement() {
    auto node = std::make_unique<IfNode>();
    consume(TokenType::LParen, "Expected ( after if");
    node->condition = expression();
    consume(TokenType::RParen, "Expected )");
    if (check(TokenType::LBrace)) {
        node->thenBlock = block();
    } else {
        node->thenBlock.push_back(statement());
    }
    if (match({TokenType::Else})) {
        if (check(TokenType::LBrace)) {
            node->elseBlock = block();
        } else {
            node->elseBlock.push_back(statement());
        }
    }
    return node;
}

std::unique_ptr<ASTNode> Parser::whileStatement() {
    auto node = std::make_unique<WhileNode>();
    consume(TokenType::LParen, "Expected ( after while");
    node->condition = expression();
    consume(TokenType::RParen, "Expected )");
    loopDepth++;
    if (check(TokenType::LBrace)) {
        node->body = block();
    } else {
        node->body.push_back(statement());
    }
    loopDepth--;
    return node;
}

std::unique_ptr<ASTNode> Parser::doWhileStatement() {
    auto node = std::make_unique<DoWhileNode>();
    loopDepth++;
    if (check(TokenType::LBrace)) {
        node->body = block();
    } else {
        node->body.push_back(statement());
    }
    consume(TokenType::While, "Expected while after do body");
    consume(TokenType::LParen, "Expected ( after while");
    node->condition = expression();
    consume(TokenType::RParen, "Expected ) after do-while condition");
    consume(TokenType::Semicolon, "Expected ; after do-while");
    loopDepth--;
    return node;
}

std::unique_ptr<ASTNode> Parser::forStatement() {
    auto node = std::make_unique<ForNode>();
    consume(TokenType::LParen, "Expected ( after for");

    if (!check(TokenType::Semicolon)) {
        if (check(TokenType::Int) || check(TokenType::Void) || check(TokenType::Float) ||
            check(TokenType::Double) || check(TokenType::Char)) {
            std::string type = advance().value;
            std::string name = consume(TokenType::Identifier, "Expected identifier").value;
            std::unique_ptr<ExprNode> initExpr = nullptr;
            if (match({TokenType::Assign})) {
                initExpr = expression();
            }
            node->init = std::make_unique<DeclarationNode>(type, name, false, false, false, std::move(initExpr));
            consume(TokenType::Semicolon, "Expected ; after for-init declaration");
        } else {
            node->init = expression();
            consume(TokenType::Semicolon, "Expected ; after for-init expression");
        }
    } else {
        consume(TokenType::Semicolon, "Expected ;");
    }

    if (!check(TokenType::Semicolon)) {
        node->condition = expression();
    }
    consume(TokenType::Semicolon, "Expected ; after for condition");

    if (!check(TokenType::RParen)) {
        node->increment = expression();
    }
    consume(TokenType::RParen, "Expected ) after for clauses");

    loopDepth++;
    node->body = block();
    loopDepth--;
    return node;
}

std::vector<std::unique_ptr<ASTNode>> Parser::block() {
    consume(TokenType::LBrace, "Expected {");
    std::vector<std::unique_ptr<ASTNode>> stmts;
    while (!check(TokenType::RBrace) && !isAtEnd()) {
        stmts.push_back(statement());
    }
    consume(TokenType::RBrace, "Expected }");
    return stmts;
}

std::unique_ptr<ExprNode> Parser::expression() {
    return assignment();
}

std::unique_ptr<ExprNode> Parser::assignment() {
    auto lhs = ternary();
    if (match({TokenType::Assign, TokenType::PlusAssign, TokenType::MinusAssign, TokenType::StarAssign,
               TokenType::SlashAssign, TokenType::PercentAssign, TokenType::XorAssign, TokenType::AndAssign,
               TokenType::OrAssign, TokenType::ShiftLeftAssign, TokenType::ShiftRightAssign})) {
        Token opToken = previous();
        auto value = assignment();
        if (auto var = dynamic_cast<VariableNode*>(lhs.get())) {
            if (opToken.type == TokenType::Assign) {
                return std::make_unique<BinaryOpNode>("=", std::move(lhs), std::move(value));
            }
            std::string baseOp;
            if (opToken.type == TokenType::PlusAssign) baseOp = "+";
            else if (opToken.type == TokenType::MinusAssign) baseOp = "-";
            else if (opToken.type == TokenType::StarAssign) baseOp = "*";
            else if (opToken.type == TokenType::SlashAssign) baseOp = "/";
            else if (opToken.type == TokenType::PercentAssign) baseOp = "%";
            else if (opToken.type == TokenType::XorAssign) baseOp = "^";
            else if (opToken.type == TokenType::AndAssign) baseOp = "&";
            else if (opToken.type == TokenType::OrAssign) baseOp = "|";
            else if (opToken.type == TokenType::ShiftLeftAssign) baseOp = "<<";
            else if (opToken.type == TokenType::ShiftRightAssign) baseOp = ">>";
            auto lhsCopy = std::make_unique<VariableNode>(var->name);
            auto combined = std::make_unique<BinaryOpNode>(baseOp, std::move(lhsCopy), std::move(value));
            return std::make_unique<BinaryOpNode>("=", std::move(lhs), std::move(combined));
        }
        throw std::runtime_error("Invalid assignment target");
    }
    return lhs;
}

std::unique_ptr<ExprNode> Parser::ternary() {
    auto cond = logicalOr();
    if (match({TokenType::Question})) {
        auto trueExpr = expression();
        consume(TokenType::Colon, "Expected : in ternary expression");
        auto falseExpr = expression();
        return std::make_unique<TernaryNode>(std::move(cond), std::move(trueExpr), std::move(falseExpr));
    }
    return cond;
}

std::unique_ptr<ExprNode> Parser::logicalOr() {
    auto expr = logicalAnd();
    while (match({TokenType::Or})) {
        auto right = logicalAnd();
        expr = std::make_unique<BinaryOpNode>("||", std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::logicalAnd() {
    auto expr = bitwiseOr();
    while (match({TokenType::And})) {
        auto right = bitwiseOr();
        expr = std::make_unique<BinaryOpNode>("&&", std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::bitwiseOr() {
    auto expr = bitwiseXor();
    while (match({TokenType::BitOr})) {
        auto right = bitwiseXor();
        expr = std::make_unique<BinaryOpNode>("|", std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::bitwiseXor() {
    auto expr = bitwiseAnd();
    while (match({TokenType::BitXor})) {
        auto right = bitwiseAnd();
        expr = std::make_unique<BinaryOpNode>("^", std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::bitwiseAnd() {
    auto expr = equality();
    while (match({TokenType::BitAnd})) {
        auto right = equality();
        expr = std::make_unique<BinaryOpNode>("&", std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::equality() {
    auto expr = comparison();
    while (match({TokenType::Eq, TokenType::Neq})) {
        std::string op = previous().value;
        auto right = comparison();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::comparison() {
    auto expr = shift();
    while (match({TokenType::Lt, TokenType::Leq, TokenType::Gt, TokenType::Geq})) {
        std::string op = previous().value;
        auto right = shift();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::shift() {
    auto expr = term();
    while (match({TokenType::ShiftLeft, TokenType::ShiftRight})) {
        std::string op = previous().value;
        auto right = term();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::term() {
    auto expr = factor();
    while (match({TokenType::Plus, TokenType::Minus})) {
        std::string op = previous().value;
        auto right = factor();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::factor() {
    auto expr = unary();
    while (match({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
        std::string op = previous().value;
        auto right = unary();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::unary() {
    if (match({TokenType::Minus, TokenType::Bang, TokenType::BitNot})) {
        std::string op = previous().value;
        return std::make_unique<UnaryOpNode>(op, unary());
    }
    return primary();
}

std::unique_ptr<ExprNode> Parser::primary() {
    if (match({TokenType::StaticCast, TokenType::DynamicCast})) {
        std::string castKind = previous().value;
        consume(TokenType::Lt, "Expected < after cast keyword");
        std::string target = consume(TokenType::Int, "Only int casts are supported").value;
        consume(TokenType::Gt, "Expected > after cast type");
        consume(TokenType::LParen, "Expected ( after cast target type");
        auto expr = expression();
        consume(TokenType::RParen, "Expected ) after cast expression");
        return std::make_unique<CastNode>(castKind, target, std::move(expr));
    }
    if (match({TokenType::LParen})) {
        if (check(TokenType::Int)) {
            std::string target = advance().value;
            consume(TokenType::RParen, "Expected ) after C-style cast type");
            auto expr = unary();
            return std::make_unique<CastNode>("c_style_cast", target, std::move(expr));
        }
        auto expr = expression();
        consume(TokenType::RParen, "Expected ) after expression");
        return expr;
    }
    if (match({TokenType::Number})) {
        return std::make_unique<IntLiteralNode>(std::stoi(previous().value));
    }
    if (match({TokenType::True})) {
        return std::make_unique<IntLiteralNode>(1);
    }
    if (match({TokenType::False})) {
        return std::make_unique<IntLiteralNode>(0);
    }
    if (match({TokenType::Identifier})) {
        std::string name = previous().value;
        if (match({TokenType::LParen})) {
            auto call = std::make_unique<FunctionCallNode>(name);
            if (!check(TokenType::RParen)) {
                do {
                    call->args.push_back(expression());
                } while (match({TokenType::Comma}));
            }
            consume(TokenType::RParen, "Expected ) after function call arguments");
            return call;
        }
        return std::make_unique<VariableNode>(name);
    }
    throw std::runtime_error("Expected expression");
}

std::unique_ptr<ASTNode> Parser::printStatement() {
    consume(TokenType::LParen, "Expected ( after print");
    auto expr = expression();
    consume(TokenType::RParen, "Expected ) after print expression");
    consume(TokenType::Semicolon, "Expected ; after print");
    return std::make_unique<PrintNode>(std::move(expr));
}