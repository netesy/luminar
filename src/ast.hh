#include <memory>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include "value.hh"

// Base Classes
class SourceLocation {
public:
    int line;
    int column;
    std::optional<std::string> filename;

    SourceLocation(int line, int column)
        : line(line), column(column) {}

    SourceLocation(int line, int column, const std::string& filename)
        : line(line), column(column), filename(filename) {}
};

class NodeMetadata {
public:
    bool isConstant = false;
    bool isPure = false;
    bool isUsed = false;
    bool synthetic = false;
    std::string documentation;

    struct ErrorDetail {
        enum class Severity {
            Warning,
            Error,
            Critical
        };

        Severity severity;
        std::string message;
        std::optional<std::string> suggestion; // Optional fix or hint
        std::optional<SourceLocation> location; // Optional source location

        ErrorDetail(Severity severity, const std::string &message,
                    std::optional<std::string> suggestion = std::nullopt,
                    std::optional<SourceLocation> location = std::nullopt)
            : severity(severity), message(message), suggestion(suggestion), location(location) {}
    };

    std::vector<ErrorDetail> errors;

    void addError(ErrorDetail::Severity severity, const std::string &message,
                  std::optional<std::string> suggestion = std::nullopt,
                  std::optional<SourceLocation> location = std::nullopt) {
        errors.emplace_back(severity, message, suggestion, location);
    }

    std::string formatErrors() const {
        std::ostringstream output;
        for (const auto &error : errors) {
            output << (error.severity == ErrorDetail::Severity::Warning ? "[Warning] "
                       : error.severity == ErrorDetail::Severity::Error   ? "[Error] "
                                                                        : "[Critical] ");
            output << error.message;
            if (error.location) {
                output << " (Line: " << error.location->line
                       << ", Column: " << error.location->column << ")";
                if (error.location->filename) {
                    output << " in file " << *error.location->filename;
                }
            }
            if (error.suggestion) {
                output << "\n  Suggestion: " << *error.suggestion;
            }
            output << "\n";
        }
        return output.str();
    }
};

class ASTNode
{
public:
    SourceLocation location;
    std::optional<NodeMetadata> metadata;

    ASTNode(SourceLocation loc)
        : location(loc)
    {}

    // Access or create metadata lazily
    NodeMetadata &getOrCreateMetadata()
    {
        if (!metadata) {
            metadata = NodeMetadata();
        }
        return *metadata;
    }

    // Check if metadata exists without creating it
    bool hasMetadata() const
    {
        return metadata.has_value();
    }

    virtual ~ASTNode() = default;
    virtual void accept(class ASTVisitor &visitor) = 0;
};

// Visitor Interface
class ASTVisitor
{
public:
    virtual void visit(class NumberNode &node) = 0;
    virtual void visit(class StringLiteralNode &node) = 0;
    virtual void visit(class BinaryNode &node) = 0;
    virtual void visit(class ConditionalNode &node) = 0;
    virtual void visit(class BlockNode &node) = 0;
    virtual void visit(class WhileNode &node) = 0;
    virtual void visit(class ForNode &node) = 0;
    virtual void visit(class RangeNode &node) = 0;
    virtual void visit(class ListNode &node) = 0;
    virtual void visit(class DictNode &node) = 0;
    virtual void visit(class ParallelNode &node) = 0;
    virtual void visit(class VariableNode &node) = 0;
    virtual void visit(class AssignmentNode &node) = 0;
    virtual void visit(class ConcurrentNode &node) = 0;
    virtual void visit(class FunctionNode &node) = 0;
    virtual void visit(class ClassNode &node) = 0;
    virtual void visit(class ModuleNode &node) = 0;
    virtual void visit(class ErrorHandlingNode &node) = 0;
    virtual void visit(class MatchNode &node) = 0;
    virtual void visit(class StreamProcessingNode &node) = 0;
    virtual void visit(class AtomicNode &node) = 0;
    virtual void visit(class ChannelNode &node) = 0;
    virtual void visit(FieldNode &node) = 0;
    virtual void visit(LambdaNode &node) = 0;
    virtual void visit(ImportNode &node) = 0;
    virtual void visit(ContractNode &node) = 0;
    virtual void visit(InterfaceNode &node) = 0;
    virtual void visit(MixinNode &node) = 0;
    virtual void visit(UnsafeNode &node) = 0;
};

// Node Implementations
class Expression : public ASTNode
{
public:
    Type type;
    Expression(SourceLocation loc, Type type)
        : ASTNode(loc)
        , type(type)
    {}
};

class Statement : public ASTNode
{
public:
    Statement(SourceLocation loc)
        : ASTNode(loc)
    {}
};

class NumberNode : public Expression
{
public:
    Value value; // From ../values.hh

    NumberNode(SourceLocation loc, Type type, Value val)
        : Expression(loc, type)
        , value(val)
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BlockNode : public Statement {
private:
    std::vector<std::unique_ptr<Statement>> statements;

public:
    BlockNode(SourceLocation loc, std::vector<std::unique_ptr<Statement>> statements)
        : Statement(loc),
        statements(std::move(statements)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class StringLiteralNode : public Expression
{
public:
    Value value; // From ../values.hh

    StringLiteralNode(SourceLocation loc, Type type, Value val)
        : Expression(loc, type)
        , value(val)
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BinaryNode : public Expression
{
public:
    std::string operatorType;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    BinaryNode(SourceLocation loc,
               Type type,
               const std::string &op,
               std::unique_ptr<Expression> lhs,
               std::unique_ptr<Expression> rhs)
        : Expression(loc, type)
        , operatorType(op)
        , left(std::move(lhs))
        , right(std::move(rhs))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ConditionalNode : public Statement
{
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::optional<std::unique_ptr<Statement>> elseBranch;

    ConditionalNode(SourceLocation loc,
                    std::unique_ptr<Expression> cond,
                    std::unique_ptr<Statement> thenBranch,
                    std::optional<std::unique_ptr<Statement>> elseBranch)
        : Statement(loc)
        , condition(std::move(cond))
        , thenBranch(std::move(thenBranch))
        , elseBranch(std::move(elseBranch))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class WhileNode : public Statement
{
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    WhileNode(SourceLocation loc, std::unique_ptr<Expression> cond, std::unique_ptr<Statement> body)
        : Statement(loc)
        , condition(std::move(cond))
        , body(std::move(body))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ForNode : public Statement
{
public:
    std::unique_ptr<ASTNode> initializer;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<ASTNode> increment;
    std::unique_ptr<Statement> body;

    ForNode(SourceLocation loc,
            std::unique_ptr<ASTNode> init,
            std::unique_ptr<Expression> cond,
            std::unique_ptr<ASTNode> incr,
            std::unique_ptr<Statement> body)
        : Statement(loc)
        , initializer(std::move(init))
        , condition(std::move(cond))
        , increment(std::move(incr))
        , body(std::move(body))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class RangeNode : public Expression
{
public:
    std::unique_ptr<Expression> start;
    std::unique_ptr<Expression> end;
    std::optional<std::unique_ptr<Expression>> step;

    RangeNode(SourceLocation loc,
              Type type,
              std::unique_ptr<Expression> start,
              std::unique_ptr<Expression> end,
              std::optional<std::unique_ptr<Expression>> step)
        : Expression(loc, type)
        , start(std::move(start))
        , end(std::move(end))
        , step(std::move(step))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ListNode : public Expression
{
public:
    std::vector<std::unique_ptr<Expression>> elements;

    ListNode(SourceLocation loc, Type type, std::vector<std::unique_ptr<Expression>> elements)
        : Expression(loc, type)
        , elements(std::move(elements))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class DictNode : public Expression
{
public:
    std::map<std::string, std::unique_ptr<Expression>> entries;

    DictNode(SourceLocation loc,
             Type type,
             std::map<std::string, std::unique_ptr<Expression>> entries)
        : Expression(loc, type)
        , entries(std::move(entries))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ParallelNode : public Statement
{
public:
    std::vector<std::unique_ptr<Statement>> branches;

    ParallelNode(SourceLocation loc, std::vector<std::unique_ptr<Statement>> branches)
        : Statement(loc)
        , branches(std::move(branches))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ConcurrentNode : public Statement
{
public:
    std::vector<std::unique_ptr<Statement>> branches;

    ConcurrentNode(SourceLocation loc, std::vector<std::unique_ptr<Statement>> branches)
        : Statement(loc)
        , branches(std::move(branches))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class VariableNode : public Expression {
private:
    std::string name;
    std::optional<std::unique_ptr<Expression>> initialValue;
    bool isMutable_;

public:
    VariableNode(SourceLocation loc,
                 Type type,
                 const std::string& name,
                 bool isMutable = true,
                 std::optional<std::unique_ptr<Expression>> initialValue = std::nullopt)
        : Expression(loc, type)
        , name(name)
        , initialValue(std::move(initialValue))
        , isMutable_(isMutable) {}

    const std::string& getName() const { return name; }
    bool isMutable() const { return isMutable_; }

    bool hasInitializer() const {
        return initialValue.has_value();
    }

    Expression* getInitializer() const {
        return initialValue ? initialValue->get() : nullptr;
    }

    void accept(ASTVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class AssignmentNode : public Statement {
private:
    std::unique_ptr<Expression> target;
    std::unique_ptr<Expression> value;

public:
    AssignmentNode(SourceLocation loc,
                   std::unique_ptr<Expression> targetExpr,
                   std::unique_ptr<Expression> valueExpr)
        : Statement(loc)
        , target(std::move(targetExpr))
        , value(std::move(valueExpr)) {}

    Expression* getTarget() const { return target.get(); }
    Expression* getValue() const { return value.get(); }

    void accept(ASTVisitor &visitor) override {
        visitor.visit(*this);
    }
};

// Function Support
class Parameter
{
public:
    std::string name;
    Type type;
    std::optional<Value> defaultValue; // From ../values.hh

    Parameter(const std::string &name,
              Type type,
              std::optional<Value> defaultValue = std::nullopt)
        : name(name)
        , type(type)
        , defaultValue(defaultValue)
    {}
};

class FunctionNode : public Statement
{
public:
    std::string name;
    Type returnType;
    std::vector<Parameter> parameters;
    std::vector<std::unique_ptr<Statement>> body;

    FunctionNode(SourceLocation loc,
                 const std::string &name,
                 Type returnType,
                 std::vector<Parameter> params,
                 std::vector<std::unique_ptr<Statement>> body)
        : Statement(loc)
        , name(name)
        , returnType(returnType)
        , parameters(std::move(params))
        , body(std::move(body))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Class Support
class ClassNode : public Statement
{
public:
    std::string name;
    std::optional<std::string> baseClass; // For inheritance
    std::vector<std::unique_ptr<Statement>> members; // Methods and fields

    ClassNode(SourceLocation loc,
              const std::string &name,
              std::optional<std::string> baseClass = std::nullopt,
              std::vector<std::unique_ptr<Statement>> members = {})
        : Statement(loc)
        , name(name)
        , baseClass(baseClass)
        , members(std::move(members))
    {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Example Nodes for Class Members
class FieldNode : public Statement
{
public:
    std::string name;
    Type type;
    std::optional<Value> initialValue; // From ../values.hh

    FieldNode(SourceLocation loc,
              const std::string &name,
              Type type,
              std::optional<Value> initialValue = std::nullopt)
        : Statement(loc)
        , name(name)
        , type(type)
        , initialValue(initialValue)
    {}

    void accept(ASTVisitor &visitor) override {
    visitor.visit(*this);

    }
};

class MethodNode : public FunctionNode
{
public:
    bool isStatic;

    MethodNode(SourceLocation loc,
               const std::string &name,
               Type returnType,
               std::vector<Parameter> params,
               std::vector<std::unique_ptr<Statement>> body,
               bool isStatic = false)
        : FunctionNode(loc, name, returnType, std::move(params), std::move(body))
        , isStatic(isStatic)
    {}

    void accept(ASTVisitor &visitor) override {
        // Delegate to the base FunctionNode's visit method
        visitor.visit(static_cast<FunctionNode&>(*this));
    }
};

// Module Support
class ModuleNode : public Statement {
public:
    std::string name;
    std::vector<std::unique_ptr<Statement>> declarations;

    ModuleNode(SourceLocation loc,
               const std::string& moduleName,
               std::vector<std::unique_ptr<Statement>> moduleDeclarations)
        : Statement(loc)
        , name(moduleName)
        , declarations(std::move(moduleDeclarations))
    {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Error Handling Node
class ErrorHandlingNode : public Expression {
public:
    enum class ErrorType {
        Optional,
        Result,
        Matching
    };

    ErrorType errorType;
    std::unique_ptr<Expression> expression;
    std::optional<std::unique_ptr<Expression>> errorHandler;

    ErrorHandlingNode(SourceLocation loc,
                      Type type,
                      ErrorType errType,
                      std::unique_ptr<Expression> expr,
                      std::optional<std::unique_ptr<Expression>> handler = std::nullopt)
        : Expression(loc, type)
        , errorType(errType)
        , expression(std::move(expr))
        , errorHandler(std::move(handler))
    {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Pattern Matching Node
class MatchNode : public Statement {
public:
    std::unique_ptr<Expression> matchExpression;
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> matchCases;
    std::optional<std::unique_ptr<Statement>> defaultCase;

    MatchNode(SourceLocation loc,
              std::unique_ptr<Expression> expr,
              std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> cases,
              std::optional<std::unique_ptr<Statement>> defaultCase = std::nullopt)
        : Statement(loc)
        , matchExpression(std::move(expr))
        , matchCases(std::move(cases))
        , defaultCase(std::move(defaultCase))
    {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Stream Processing Node
class StreamProcessingNode : public Statement {
public:
    std::unique_ptr<Expression> inputStream;
    std::unique_ptr<Expression> processingFunction;
    std::unique_ptr<Expression> outputChannel;

    StreamProcessingNode(SourceLocation loc,
                         std::unique_ptr<Expression> input,
                         std::unique_ptr<Expression> processFunc,
                         std::unique_ptr<Expression> output)
        : Statement(loc)
        , inputStream(std::move(input))
        , processingFunction(std::move(processFunc))
        , outputChannel(std::move(output))
    {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Atomic Operation Node
class AtomicNode : public Expression {
public:
    enum class Operation {
        FetchAdd,
        CompareExchange,
        Load,
        Store
    };

    Operation op;
    std::unique_ptr<Expression> target;
    std::optional<std::unique_ptr<Expression>> value;

    AtomicNode(SourceLocation loc,
               Type type,
               Operation operation,
               std::unique_ptr<Expression> target,
               std::optional<std::unique_ptr<Expression>> value = std::nullopt)
        : Expression(loc, type)
        , op(operation)
        , target(std::move(target))
        , value(std::move(value))
    {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Channel Communication Node
class ChannelNode : public Statement {
public:
    enum class ChannelOperation {
        Send,
        Receive,
        Collect
    };

    ChannelOperation op;
    std::unique_ptr<Expression> channel;
    std::optional<std::unique_ptr<Expression>> data;

    ChannelNode(SourceLocation loc,
                ChannelOperation operation,
                std::unique_ptr<Expression> channelExpr,
                std::optional<std::unique_ptr<Expression>> channelData = std::nullopt)
        : Statement(loc)
        , op(operation)
        , channel(std::move(channelExpr))
        , data(std::move(channelData))
    {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class LambdaNode : public Expression {
public:
    std::vector<Parameter> parameters;
    std::unique_ptr<Statement> body;

    LambdaNode(SourceLocation loc, Type type, std::vector<Parameter> params, std::unique_ptr<Statement> body)
        : Expression(loc, type), parameters(std::move(params)), body(std::move(body)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ImportNode : public Statement {
public:
    std::string moduleName;
    std::optional<std::string> alias;

    ImportNode(SourceLocation loc, const std::string &moduleName, std::optional<std::string> alias = std::nullopt)
        : Statement(loc), moduleName(moduleName), alias(alias) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// New Node for Contract
class ContractNode : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::string message;
    std::string action;

    ContractNode(SourceLocation loc, std::unique_ptr<Expression> cond, const std::string &msg, const std::string &act)
        : Statement(loc), condition(std::move(cond)), message(msg), action(act) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// New Node for Interface
class InterfaceNode : public Statement {
public:
    std::string name;
    std::vector<std::unique_ptr<Statement>> methods;

    InterfaceNode(SourceLocation loc, const std::string &name, std::vector<std::unique_ptr<Statement>> methods)
        : Statement(loc), name(name), methods(std::move(methods)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// New Node for Mixin
class MixinNode : public Statement {
public:
    std::string name;
    std::vector<std::unique_ptr<Statement>> methods;

    MixinNode(SourceLocation loc, const std::string &name, std::vector<std::unique_ptr<Statement>> methods)
        : Statement(loc), name(name), methods(std::move(methods)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// New Node for Unsafe Block
class UnsafeNode : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> body;

    UnsafeNode(SourceLocation loc, std::vector<std::unique_ptr<Statement>> body)
        : Statement(loc), body(std::move(body)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};
