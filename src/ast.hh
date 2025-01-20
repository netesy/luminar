#pragma once

#include "token.hh"
#include "value.hh" // Include the value system
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef AST_HH
#define AST_HH

// Forward declarations
class ASTVisitor;

// Enhanced source location with more detail
struct SourceLocation {
    size_t startOffset;
    size_t endOffset;
    size_t line;
    size_t column;
    std::string filename;
};


// Enhanced metadata for analysis
struct NodeMetadata {
    bool isConstant = false;
    bool isPure = false;
    bool isUsed = false;
    bool synthetic = false;
    std::string documentation;
    std::optional<std::string> errorMsg;
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual TypeTag getType() const = 0;
    virtual void accept(ASTVisitor& visitor) = 0;
    // Location management
    virtual SourceLocation getLocation() const { return location; }
    void setLocation(SourceLocation loc) { location = loc; }
    // Tree structure
    void setParent(std::shared_ptr<ASTNode> p) { parent = p; }
    std::shared_ptr<ASTNode> getParent() const { return parent.lock(); }

protected:
    SourceLocation location;
    // NodeMetadata metadata;
    // TypePtr inferredType;
    std::weak_ptr<ASTNode> parent;
    // uint32_t scopeLevel = 0;
};

// Literal Nodes
class NumberNode : public ASTNode {
private:
    ValuePtr value;

public:
    NumberNode(int64_t num) {
        auto type = std::make_shared<Type>(TypeTag::Int64);
        value = std::make_shared<Value>(Value{type, num});
    }

    NumberNode(double num) {
        auto type = std::make_shared<Type>(TypeTag::Float64);
        value = std::make_shared<Value>(Value{type, num});
    }

    TypeTag getType() const override { return value->type->tag; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    ValuePtr getValue() const { return value; }
};

class StringLiteralNode : public ASTNode {
private:
    ValuePtr value;

public:
    StringLiteralNode(const std::string& str) {
        auto type = std::make_shared<Type>(TypeTag::String);
        value = std::make_shared<Value>(Value{type, str});
    }

    TypeTag getType() const override { return TypeTag::String; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    ValuePtr getValue() const { return value; }
};


class LiteralNode : public ASTNode {
private:
    std::variant<int, double, std::string, bool> value;

public:
    explicit LiteralNode(const Value value)
        : value(value) {}

    TypeTag getType() const override { return TypeTag::Literal; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    const std::variant<int, double, std::string, bool>& getValue() const { return value; }
};


// Expression Nodes
class BinaryNode : public ASTNode {
private:
    TokenType op;
    std::unique_ptr<ASTNode> left, right;

public:
    BinaryNode(TokenType op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    TypeTag getType() const override { return TypeTag::Any; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    TokenType getOp() const { return op; }
    ASTNode* getLeft() const { return left.get(); }
    ASTNode* getRight() const { return right.get(); }
};

class UnaryExprNode : public ASTNode {
private:
    TokenType op;
    std::unique_ptr<ASTNode> expr;

public:
    UnaryExprNode(TokenType op, std::unique_ptr<ASTNode> expr)
        : op(op), expr(std::move(expr)) {}

    TypeTag getType() const override { return TypeTag::Any; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    TokenType getOp() const { return op; }
    ASTNode* getExpr() const { return expr.get(); }
};

class LogicalExprNode : public ASTNode {
private:
    TokenType op;
    std::unique_ptr<ASTNode> left, right;

public:
    LogicalExprNode(TokenType op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    TypeTag getType() const override { return TypeTag::Logical; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Control Flow Nodes
class ConditionalNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> thenBranch;
    std::unique_ptr<ASTNode> elseBranch;

public:
    ConditionalNode(std::unique_ptr<ASTNode> condition,
                    std::unique_ptr<ASTNode> thenBranch,
                    std::unique_ptr<ASTNode> elseBranch)
        : condition(std::move(condition)),
        thenBranch(std::move(thenBranch)),
        elseBranch(std::move(elseBranch)) {}

    TypeTag getType() const override { return TypeTag::Conditional; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class WhileNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

public:
    WhileNode(std::unique_ptr<ASTNode> condition, std::unique_ptr<ASTNode> body)
        : condition(std::move(condition)), body(std::move(body)) {}

    TypeTag getType() const override { return TypeTag::While; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ForNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> iterator;
    std::unique_ptr<ASTNode> body;

public:
    ForNode(std::unique_ptr<ASTNode> iterator, std::unique_ptr<ASTNode> body)
        : iterator(std::move(iterator)), body(std::move(body)) {}

    TypeTag getType() const override { return TypeTag::For; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Functionality Nodes
class FunctionNode : public ASTNode {
private:
    std::string name;
    std::vector<std::pair<std::string, TypePtr>> parameters;
    TypePtr returnType;
    std::unique_ptr<ASTNode> body;

public:
    FunctionNode(const std::string& name,
                 std::vector<std::pair<std::string, TypePtr>> parameters,
                 TypePtr returnType,
                 std::unique_ptr<ASTNode> body)
        : name(name),
        parameters(std::move(parameters)),
        returnType(std::move(returnType)),
        body(std::move(body)) {}

    TypeTag getType() const override { return TypeTag::Function; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Function Call Node
class FunctionCallNode : public ASTNode {
private:
    std::string callee;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    FunctionCallNode(const std::string& callee, std::vector<std::unique_ptr<ASTNode>> args)
        : callee(callee), arguments(std::move(args)) {}

    TypeTag getType() const override { return TypeTag::Function; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    const std::string& getCallee() const { return callee; }
    const std::vector<std::unique_ptr<ASTNode>>& getArguments() const { return arguments; }
};


class VariableNode : public ASTNode {
private:
    std::string name;
    std::optional<std::string> type; // Optional type annotation

public:
    VariableNode(const std::string& name, std::optional<std::string> type = std::nullopt)
        : name(name), type(type) {}

    TypeTag getType() const override { return TypeTag::Any; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    const std::string& getName() const { return name; }
    const std::optional<std::string>& getTypeAnnotation() const { return type; }
};



// Assignment Nodes
class AssignmentNode : public ASTNode {
private:
    std::unique_ptr<VariableNode> variable;
    std::unique_ptr<ASTNode> value;

public:
    AssignmentNode(std::unique_ptr<VariableNode> variable, std::unique_ptr<ASTNode> value)
        : variable(std::move(variable)), value(std::move(value)) {}

    TypeTag getType() const override { return TypeTag::Any; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    VariableNode* getVariable() const { return variable.get(); }
    ASTNode* getValue() const { return value.get(); }
};


// Class Nodes
class ClassNode : public ASTNode {
private:
    std::string name;
    std::vector<std::unique_ptr<VariableNode>> members;
    std::vector<std::unique_ptr<ASTNode>> methods;

public:
    ClassNode(const std::string& name)
        : name(name) {}

    TypeTag getType() const override { return TypeTag::Class; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    const std::string& getName() const { return name; }
    void addMember(std::unique_ptr<VariableNode> member) { members.push_back(std::move(member)); }
    void addMethod(std::unique_ptr<ASTNode> method) { methods.push_back(std::move(method)); }
    const std::vector<std::unique_ptr<VariableNode>>& getMembers() const { return members; }
    const std::vector<std::unique_ptr<ASTNode>>& getMethods() const { return methods; }
};

// Method Nodes
class MethodNode : public ASTNode {
private:
    std::string name;
    std::vector<std::pair<std::string, TypePtr>> parameters;
    TypePtr returnType;
    std::unique_ptr<ASTNode> body;

public:
    MethodNode(const std::string& name,
               std::vector<std::pair<std::string, TypePtr>> parameters,
               TypePtr returnType,
               std::unique_ptr<ASTNode> body)
        : name(name), parameters(std::move(parameters)), returnType(std::move(returnType)), body(std::move(body)) {}

    TypeTag getType() const override { return TypeTag::Method; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    const std::string& getName() const { return name; }
    const std::vector<std::pair<std::string, TypePtr>>& getParameters() const { return parameters; }
    TypePtr getReturnType() const { return returnType; }
    ASTNode* getBody() const { return body.get(); }
};

// Additional Constructs
class RangeNode : public ASTNode {
private:
    std::unique_ptr<ASTNode> start;
    std::unique_ptr<ASTNode> end;
    std::unique_ptr<ASTNode> step;

public:
    RangeNode(std::unique_ptr<ASTNode> start, std::unique_ptr<ASTNode> end, std::unique_ptr<ASTNode> step = nullptr)
        : start(std::move(start)), end(std::move(end)), step(std::move(step)) {}

    TypeTag getType() const override { return TypeTag::Range; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};


// Base visitor interface
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    // Core expressions
    virtual void visit(NumberNode& node) = 0;
    virtual void visit(BinaryNode& node) = 0;
    virtual void visit(UnaryExprNode& node) = 0; // Corrected name
    virtual void visit(StringLiteralNode& node) = 0;
    // virtual void visit(ListNode& node) = 0;
    virtual void visit(VariableNode& node) = 0; // Added
    virtual void visit(AssignmentNode& node) = 0; // Added

    // Functions and calls
    virtual void visit(FunctionNode& node) = 0;
    // virtual void visit(CallNode& node) = 0;
    // virtual void visit(ReturnNode& node) = 0;

    // Control flow
    virtual void visit(ConditionalNode& node) = 0;
    virtual void visit(WhileNode& node) = 0;
    virtual void visit(ForNode& node) = 0; // Added
    // virtual void visit(BlockNode& node) = 0; // Added
    // virtual void visit(PatternMatchNode& node) = 0;

    // Other
    // virtual void visit(EnumNode& node) = 0;
    // virtual void visit(ParallelNode& node) = 0;
    // virtual void visit(ConcurrentNode& node) = 0;
    virtual void visit(RangeNode& node) = 0;
    virtual void visit(LogicalExprNode& node) = 0;

};
#endif



#pragma once

#include "token.hh"
#include "value.hh"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef AST_HH
#define AST_HH

// Forward declarations
class ASTVisitor;

// Enhanced source location with more detail
struct SourceLocation {
    size_t startOffset;
    size_t endOffset;
    size_t line;
    size_t column;
    std::string filename;
};

// Enhanced metadata for analysis
struct NodeMetadata {
    bool isConstant = false;
    bool isPure = false;
    bool isUsed = false;
    bool synthetic = false;
    std::string documentation;
    std::optional<std::string> errorMsg;
};

// Base AST Node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual TypeTag getType() const = 0;
    virtual void accept(ASTVisitor& visitor) = 0;

    // Location management
    virtual SourceLocation getLocation() const { return location; }
    void setLocation(SourceLocation loc) { location = loc; }

    // Tree structure
    void setParent(std::shared_ptr<ASTNode> p) { parent = p; }
    std::shared_ptr<ASTNode> getParent() const { return parent.lock(); }

protected:
    SourceLocation location;
    NodeMetadata metadata;
    std::weak_ptr<ASTNode> parent;
};

// Expression Base Class
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
    virtual TypePtr getInferredType() const { return inferredType; }
    void setInferredType(TypePtr type) { inferredType = type; }

protected:
    TypePtr inferredType;
};

// Statement Base Class
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
    virtual bool isTerminator() const { return false; }
};

// Literal Expressions
class LiteralExpression : public Expression {
protected:
    ValuePtr value;

public:
    ValuePtr getValue() const { return value; }
};

class NumberNode : public LiteralExpression {
public:
    NumberNode(int64_t num) {
        auto type = std::make_shared<Type>(TypeTag::Int64);
        value = std::make_shared<Value>(Value{type, num});
    }

    NumberNode(double num) {
        auto type = std::make_shared<Type>(TypeTag::Float64);
        value = std::make_shared<Value>(Value{type, num});
    }

    TypeTag getType() const override { return value->type->tag; }
    void accept(ASTVisitor& visitor) override;
};

class StringLiteralNode : public LiteralExpression {
public:
    StringLiteralNode(const std::string& str) {
        auto type = std::make_shared<Type>(TypeTag::String);
        value = std::make_shared<Value>(Value{type, str});
    }

    TypeTag getType() const override { return TypeTag::String; }
    void accept(ASTVisitor& visitor) override;
};

// Expression Nodes
class BinaryNode : public Expression {
private:
    TokenType op;
    std::unique_ptr<Expression> left, right;

public:
    BinaryNode(TokenType op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}

    TypeTag getType() const override { return TypeTag::Any; }
    void accept(ASTVisitor& visitor) override;

    TokenType getOp() const { return op; }
    Expression* getLeft() const { return left.get(); }
    Expression* getRight() const { return right.get(); }
};

// Statement Nodes
class ExpressionStatement : public Statement {
private:
    std::unique_ptr<Expression> expr;

public:
    ExpressionStatement(std::unique_ptr<Expression> expr)
        : expr(std::move(expr)) {}

    TypeTag getType() const override { return TypeTag::Statement; }
    void accept(ASTVisitor& visitor) override;
    Expression* getExpression() const { return expr.get(); }
};

class Block : public Statement {
private:
    std::vector<std::unique_ptr<Statement>> statements;

public:
    TypeTag getType() const override { return TypeTag::Block; }
    void accept(ASTVisitor& visitor) override;

    void addStatement(std::unique_ptr<Statement> stmt) {
        statements.push_back(std::move(stmt));
    }

    const std::vector<std::unique_ptr<Statement>>& getStatements() const {
        return statements;
    }
};

// Declaration Nodes
class Declaration : public Statement {
protected:
    std::string name;
    TypePtr declaredType;

public:
    Declaration(const std::string& name, TypePtr type)
        : name(name), declaredType(type) {}

    const std::string& getName() const { return name; }
    TypePtr getDeclaredType() const { return declaredType; }
};

// Enhanced Visitor Pattern
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    // Expressions
    virtual void visit(NumberNode& node) = 0;
    virtual void visit(StringLiteralNode& node) = 0;
    virtual void visit(BinaryNode& node) = 0;
    virtual void visit(UnaryExprNode& node) = 0;
    virtual void visit(VariableNode& node) = 0;

    // Statements
    virtual void visit(ExpressionStatement& node) = 0;
    virtual void visit(Block& node) = 0;
    virtual void visit(ConditionalNode& node) = 0;
    virtual void visit(WhileNode& node) = 0;
    virtual void visit(ForNode& node) = 0;

    // Declarations
    virtual void visit(FunctionNode& node) = 0;
    virtual void visit(ClassNode& node) = 0;
    virtual void visit(MethodNode& node) = 0;
    virtual void visit(VariableDeclaration& node) = 0;

    // Other
    virtual void visit(RangeNode& node) = 0;
    virtual void visit(LogicalExprNode& node) = 0;
};

// Abstract Visitor Implementation
class AbstractASTVisitor : public ASTVisitor {
public:
    // Default implementations that do nothing
    void visit(NumberNode& node) override {}
    void visit(StringLiteralNode& node) override {}
    void visit(BinaryNode& node) override {}
    void visit(UnaryExprNode& node) override {}
    void visit(VariableNode& node) override {}
    void visit(ExpressionStatement& node) override {}
    void visit(Block& node) override {}
    void visit(ConditionalNode& node) override {}
    void visit(WhileNode& node) override {}
    void visit(ForNode& node) override {}
    void visit(FunctionNode& node) override {}
    void visit(ClassNode& node) override {}
    void visit(MethodNode& node) override {}
    void visit(VariableDeclaration& node) override {}
    void visit(RangeNode& node) override {}
    void visit(LogicalExprNode& node) override {}
};

#endif
