#ifndef ASTNODES_H
#define ASTNODES_H

#include <vector>
#include <string>
#include <memory>
#include <map>

// Բոլոր Node-երի բազային դասը
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

// --- Expressions (Արժեք վերադարձնող հանգույցներ) ---

class ExprNode : public ASTNode {};

class IntLiteralNode : public ExprNode {
public:
    int value;
    IntLiteralNode(int v) : value(v) {}
};

class VariableNode : public ExprNode {
public:
    std::string name;
    VariableNode(const std::string& n) : name(n) {}
};

class FunctionCallNode : public ExprNode {
public:
    std::string callee;
    std::vector<std::unique_ptr<ExprNode>> args;
    FunctionCallNode(std::string fn) : callee(std::move(fn)) {}
};

class BinaryOpNode : public ExprNode {
public:
    std::string op;
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
    BinaryOpNode(std::string o, std::unique_ptr<ExprNode> l, std::unique_ptr<ExprNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

class UnaryOpNode : public ExprNode {
public:
    std::string op;
    std::unique_ptr<ExprNode> operand;
    UnaryOpNode(std::string o, std::unique_ptr<ExprNode> expr)
        : op(o), operand(std::move(expr)) {}
};

class TernaryNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<ExprNode> trueExpr;
    std::unique_ptr<ExprNode> falseExpr;
    TernaryNode(std::unique_ptr<ExprNode> c, std::unique_ptr<ExprNode> t, std::unique_ptr<ExprNode> f)
        : condition(std::move(c)), trueExpr(std::move(t)), falseExpr(std::move(f)) {}
};

class CastNode : public ExprNode {
public:
    std::string castType; 
    std::string targetType;
    std::unique_ptr<ExprNode> expression;
    CastNode(std::string ct, std::string tt, std::unique_ptr<ExprNode> expr)
        : castType(ct), targetType(tt), expression(std::move(expr)) {}
};

class LambdaNode : public ExprNode {
public:
    std::vector<std::string> captures;
    std::vector<std::pair<std::string, std::string>> params;
    std::vector<std::unique_ptr<ASTNode>> body;
};

// --- Statements (Գործողություն կատարող հանգույցներ) ---

class StmtNode : public ASTNode {};

/** * ԱՎԵԼԱՑՎԱԾ Է: AssignmentNode
 * Սա անհրաժեշտ է Parser.cpp-ի "undefined identifier" սխալը վերացնելու համար:
 */
struct AssignmentNode : public ASTNode {
    std::string varName;
    std::unique_ptr<ExprNode> expression;
    AssignmentNode(std::string name, std::unique_ptr<ExprNode> expr)
        : varName(name), expression(std::move(expr)) {}
};

class DeclarationNode : public StmtNode {
public:
    std::string type;
    std::string name;
    bool isStatic;
    bool isGlobal;
    bool isExtern;
    std::unique_ptr<ExprNode> initializer;
    DeclarationNode(std::string t, std::string n, bool s = false, bool g = false, bool e = false, std::unique_ptr<ExprNode> init = nullptr)
        : type(t), name(n), isStatic(s), isGlobal(g), isExtern(e), initializer(std::move(init)) {}
};

/** * ԱՎԵԼԱՑՎԱԾ Է: ReturnNode
 * Ֆունկցիաներից արժեք վերադարձնելու համար:
 */
class ReturnNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expression;
    ReturnNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}
};

class BreakNode : public StmtNode {
public:
    BreakNode() = default;
};

class ContinueNode : public StmtNode {
public:
    ContinueNode() = default;
};

class PrintNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expression;
    PrintNode(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}
};

class IfNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::vector<std::unique_ptr<ASTNode>> thenBlock;
    std::vector<std::unique_ptr<ASTNode>> elseBlock;
};

class WhileNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;
};

class DoWhileNode : public StmtNode {
public:
    std::vector<std::unique_ptr<ASTNode>> body;
    std::unique_ptr<ExprNode> condition;
};

class ForNode : public StmtNode {
public:
    std::unique_ptr<ASTNode> init;
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<ExprNode> increment;
    std::vector<std::unique_ptr<ASTNode>> body;
};

class SwitchNode : public StmtNode {
public:
    struct Section {
        bool isDefault = false;
        int caseValue = 0;
        std::vector<std::unique_ptr<ASTNode>> statements;
    };

    std::unique_ptr<ExprNode> condition;
    std::map<int, std::vector<std::unique_ptr<ASTNode>>> cases;
    std::vector<std::unique_ptr<ASTNode>> defaultCase;
    std::vector<Section> orderedSections;
};

// --- High-level Structures ---

class EnumNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> values;
};

class MemberNode {
public:
    std::string accessSpecifier; 
    std::unique_ptr<ASTNode> member; 
};

class ClassNode : public ASTNode {
public:
    std::string name;
    std::string baseClass; 
    std::vector<MemberNode> members;
    bool isStruct; 
};

class UnionNode : public ASTNode {
public:
    std::string name;
    std::vector<std::pair<std::string, std::string>> members; 
};

class FunctionNode : public ASTNode {
public:
    std::string returnType;
    std::string name;
    std::vector<std::pair<std::string, std::string>> params;
    std::vector<std::unique_ptr<ASTNode>> body;
};

#endif