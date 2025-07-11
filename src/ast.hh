#ifndef LUMINAR_AST_HH
#define LUMINAR_AST_HH

#include "token.hh"
#include "types.hh"
#include "value.hh"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

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
        std::optional<std::string> suggestion;
        std::optional<SourceLocation> location;

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
        std::string result;
        for (const auto &error : errors) {
            std::string prefix;
            switch (error.severity) {
            case ErrorDetail::Severity::Warning: prefix = "[Warning] "; break;
            case ErrorDetail::Severity::Error: prefix = "[Error] "; break;
            case ErrorDetail::Severity::Critical: prefix = "[Critical] "; break;
            }

            result += prefix + error.message;

            if (error.location) {
                result += " (Line: " + std::to_string(error.location->line) +
                          ", Column: " + std::to_string(error.location->column);
                if (error.location->filename) {
                    result += " in file " + *error.location->filename;
                }
                result += ")";
            }

            if (error.suggestion) {
                result += "\n  Suggestion: " + *error.suggestion;
            }

            result += "\n";
        }
        return result;
    }
};

// Visitor Interface
class ASTVisitor {
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
    virtual void visit(class FieldNode &node) = 0;
    virtual void visit(class LambdaNode &node) = 0;
    virtual void visit(class ImportNode &node) = 0;
    virtual void visit(class InterfaceNode &node) = 0;
    virtual void visit(class MixinNode &node) = 0;
    virtual void visit(class UnsafeNode &node) = 0;
    virtual void visit(class InterpolatedStringNode &node) = 0;
    virtual void visit(class BooleanNode &node) = 0;
    virtual void visit(class UnaryNode &node) = 0;
    virtual void visit(class CallNode &node) = 0;
    virtual void visit(class GroupingNode &node) = 0;
    virtual void visit(class ReturnNode &node) = 0;
    virtual void visit(class NilNode &node) = 0;
};

class ASTNode {
public:
    SourceLocation location;
    std::optional<NodeMetadata> metadata;

    ASTNode(SourceLocation loc)
        : location(loc) {}

    // Make ASTNode move-constructible and move-assignable
    ASTNode(ASTNode&&) = default;
    ASTNode& operator=(ASTNode&&) = default;

    // Disable copying
    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;

    // Access or create metadata lazily
    NodeMetadata &getOrCreateMetadata() {
        if (!metadata) {
            metadata = NodeMetadata();
        }
        return *metadata;
    }

    // Check if metadata exists without creating it
    bool hasMetadata() const {
        return metadata.has_value();
    }

    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor &visitor) = 0;

    // Add type-safe dynamic casting method
    template<typename T>
    T* as() {
        return dynamic_cast<T*>(this);
    }

    template<typename T>
    const T* as() const {
        return dynamic_cast<const T*>(this);
    }

    // Type inference (default implementation returns ANY_TYPE)
    virtual TypePtr inferType(TypeSystem &typeSystem) {
        return typeSystem.ANY_TYPE;
    }

    // Type checking (default implementation checks inferred type against expected type)
    virtual bool typeCheck(TypeSystem &typeSystem, TypePtr expectedType) {
        TypePtr inferredType = inferType(typeSystem);
        return typeSystem.isCompatible(inferredType, expectedType);
    }

    // Recursively type-check child nodes (default implementation does nothing)
    virtual void typeCheckChildren(TypeSystem &typeSystem) {
        // Default implementation does nothing
    }
};

// Node Implementations
class Expression : public ASTNode {
public:
    Type type;
    Expression(SourceLocation loc, Type type)
        : ASTNode(loc), type(type) {}

    // Enable move semantics
    Expression(Expression&&) = default;
    Expression& operator=(Expression&&) = default;

    // Disable copying
    Expression(const Expression&) = delete;
    Expression& operator=(const Expression&) = delete;
};

class Statement : public ASTNode {
public:
    Statement(SourceLocation loc)
        : ASTNode(loc) {}

    // Enable move semantics
    Statement(Statement&&) = default;
    Statement& operator=(Statement&&) = default;

    // Disable copying
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
};

class NumberNode : public Expression {
public:
    Value value; // From ../values.hh

    NumberNode(SourceLocation loc, Type type, Value val)
        : Expression(loc, type), value(std::move(val)) {}

    // Enable move semantics
    NumberNode(NumberNode&&) = default;
    NumberNode& operator=(NumberNode&&) = default;

    // Disable copying
    NumberNode(const NumberNode&) = delete;
    NumberNode& operator=(const NumberNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BlockNode : public Statement {
private:
    std::vector<std::unique_ptr<Statement>> statements;

public:
    std::vector<std::unique_ptr<Statement>> &getStatements() { return statements; }
    const std::vector<std::unique_ptr<Statement>> &getStatements() const { return statements; }

    BlockNode(SourceLocation loc, std::vector<std::unique_ptr<Statement>> statements)
        : Statement(loc), statements(std::move(statements)) {}

    // Enable move semantics
    BlockNode(BlockNode&&) = default;
    BlockNode& operator=(BlockNode&&) = default;

    // Disable copying
    BlockNode(const BlockNode&) = delete;
    BlockNode& operator=(const BlockNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class StringLiteralNode : public Expression {
public:
    Value value; // From ../values.hh

    StringLiteralNode(SourceLocation loc, Type type, Value val)
        : Expression(loc, type), value(std::move(val)) {}

    // Enable move semantics
    StringLiteralNode(StringLiteralNode&&) = default;
    StringLiteralNode& operator=(StringLiteralNode&&) = default;

    // Disable copying
    StringLiteralNode(const StringLiteralNode&) = delete;
    StringLiteralNode& operator=(const StringLiteralNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BooleanNode : public Expression {
public:
    Value value;

    BooleanNode(SourceLocation loc, Type type, Value val)
        : Expression(loc, type), value(std::move(val)) {}

    // Enable move semantics
    BooleanNode(BooleanNode&&) = default;
    BooleanNode& operator=(BooleanNode&&) = default;

    // Disable copying
    BooleanNode(const BooleanNode&) = delete;
    BooleanNode& operator=(const BooleanNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BinaryNode : public Expression {
public:
    std::string operatorType;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    // Enable move semantics
    BinaryNode(BinaryNode&&) = default;
    BinaryNode& operator=(BinaryNode&&) = default;

    // Disable copying
    BinaryNode(const BinaryNode&) = delete;
    BinaryNode& operator=(const BinaryNode&) = delete;

    BinaryNode(SourceLocation loc, Type type, std::string op,
               std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
        : Expression(loc, type), operatorType(std::move(op)), left(std::move(lhs)), right(std::move(rhs)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    // Override inferType
    TypePtr inferType(TypeSystem &typeSystem) override {
        TypePtr leftType = left->inferType(typeSystem);
        TypePtr rightType = right->inferType(typeSystem);

        if (!typeSystem.isCompatible(leftType, rightType)) {
            getOrCreateMetadata().addError(
                NodeMetadata::ErrorDetail::Severity::Error,
                "Incompatible types in binary operation: " + leftType->toString() + " and " + rightType->toString()
                );
            return typeSystem.ANY_TYPE; // Fallback to Any type in case of error
        }

        // Return the common type of the operands
        return typeSystem.getCommonType(leftType, rightType);
    }

    // Override typeCheckChildren to recursively type-check left and right
    void typeCheckChildren(TypeSystem &typeSystem) override {
        left->typeCheckChildren(typeSystem);
        right->typeCheckChildren(typeSystem);
    }
};

class ConditionalNode : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> elifBranches;
    std::optional<std::unique_ptr<Statement>> elseBranch;

    ConditionalNode(SourceLocation loc, std::unique_ptr<Expression> cond,
                    std::unique_ptr<Statement> thenBranch,
                    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> elifBranches,
                    std::optional<std::unique_ptr<Statement>> elseBranch)
        : Statement(loc), condition(std::move(cond)), thenBranch(std::move(thenBranch)),
        elifBranches(std::move(elifBranches)), elseBranch(std::move(elseBranch)) {}

    // Enable move semantics
    ConditionalNode(ConditionalNode&&) = default;
    ConditionalNode& operator=(ConditionalNode&&) = default;

    // Disable copying
    ConditionalNode(const ConditionalNode&) = delete;
    ConditionalNode& operator=(const ConditionalNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class WhileNode : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    WhileNode(SourceLocation loc, std::unique_ptr<Expression> cond, std::unique_ptr<Statement> body)
        : Statement(loc), condition(std::move(cond)), body(std::move(body)) {}

    // Enable move semantics
    WhileNode(WhileNode&&) = default;
    WhileNode& operator=(WhileNode&&) = default;

    // Disable copying
    WhileNode(const WhileNode&) = delete;
    WhileNode& operator=(const WhileNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ForNode : public Statement {
public:
    std::unique_ptr<ASTNode> initializer;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<ASTNode> increment;
    std::unique_ptr<Statement> body;

    ForNode(SourceLocation loc, std::unique_ptr<ASTNode> init, std::unique_ptr<Expression> cond,
            std::unique_ptr<ASTNode> incr, std::unique_ptr<Statement> body)
        : Statement(loc), initializer(std::move(init)), condition(std::move(cond)),
        increment(std::move(incr)), body(std::move(body)) {}

    // Enable move semantics
    ForNode(ForNode&&) = default;
    ForNode& operator=(ForNode&&) = default;

    // Disable copying
    ForNode(const ForNode&) = delete;
    ForNode& operator=(const ForNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class RangeNode : public Expression {
public:
    std::unique_ptr<Expression> start;
    std::unique_ptr<Expression> end;
    std::unique_ptr<Expression> step;
    bool hasStep;

    RangeNode(SourceLocation loc,
              std::unique_ptr<Expression> start,
              std::unique_ptr<Expression> end,
              std::unique_ptr<Expression> step = nullptr)
        : Expression(loc, Type{TypeTag::Range}),
          start(std::move(start)),
          end(std::move(end)),
          step(std::move(step)),
          hasStep(step != nullptr) {}

    void accept(ASTVisitor &visitor) override {
        visitor.visit(*this);
    }

    // Type checking for range expressions
    TypePtr inferType(TypeSystem &typeSystem) override {
        // Check that start, end, and step (if present) have compatible numeric types
        TypePtr startType = start->inferType(typeSystem);
        TypePtr endType = end->inferType(typeSystem);

        if (!typeSystem.isNumericType(startType->tag) || !typeSystem.isNumericType(endType->tag)) {
            throw std::runtime_error("Range bounds must be numeric types");
        }

        if (hasStep) {
            TypePtr stepType = step->inferType(typeSystem);
            if (!typeSystem.isNumericType(stepType->tag)) {
                throw std::runtime_error("Step value must be a numeric type");
            }
        }

        // The range itself has a Range type
        return std::make_shared<Type>(TypeTag::Range);
    }

    // Type check children expressions
    void typeCheckChildren(TypeSystem &typeSystem) override {
        start->typeCheckChildren(typeSystem);
        end->typeCheckChildren(typeSystem);
        if (hasStep) {
            step->typeCheckChildren(typeSystem);
        }
    }
};

class ListNode : public Expression {
public:
    std::vector<std::unique_ptr<Expression>> elements;

    ListNode(SourceLocation loc, Type type, std::vector<std::unique_ptr<Expression>> elements)
        : Expression(loc, type), elements(std::move(elements)) {}

    // Enable move semantics
    ListNode(ListNode&&) = default;
    ListNode& operator=(ListNode&&) = default;

    // Disable copying
    ListNode(const ListNode&) = delete;
    ListNode& operator=(const ListNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class DictNode : public Expression {
public:
    std::map<std::string, std::unique_ptr<Expression>> entries;

    DictNode(SourceLocation loc, Type type, std::map<std::string, std::unique_ptr<Expression>> entries)
        : Expression(loc, type), entries(std::move(entries)) {}

    // Enable move semantics
    DictNode(DictNode&&) = default;
    DictNode& operator=(DictNode&&) = default;

    // Disable copying
    DictNode(const DictNode&) = delete;
    DictNode& operator=(const DictNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ConcurrentNode : public Statement {
public:
    std::unique_ptr<Expression> expression;
    std::unique_ptr<Statement> body;

    ConcurrentNode(std::unique_ptr<Expression> expr, std::unique_ptr<Statement> body)
        : Statement(SourceLocation(0, 0)), expression(std::move(expr)), body(std::move(body)) {}

    // Enable move semantics
    ConcurrentNode(ConcurrentNode&&) = default;
    ConcurrentNode& operator=(ConcurrentNode&&) = default;

    // Disable copying
    ConcurrentNode(const ConcurrentNode&) = delete;
    ConcurrentNode& operator=(const ConcurrentNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ParallelNode : public Statement {
public:
    std::unique_ptr<Expression> expression;
    std::unique_ptr<Statement> body;

    ParallelNode(std::unique_ptr<Expression> expr, std::unique_ptr<Statement> body)
        : Statement(SourceLocation(0, 0)), expression(std::move(expr)), body(std::move(body)) {}

    // Enable move semantics
    ParallelNode(ParallelNode&&) = default;
    ParallelNode& operator=(ParallelNode&&) = default;

    // Disable copying
    ParallelNode(const ParallelNode&) = delete;
    ParallelNode& operator=(const ParallelNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class VariableNode : public Expression {
private:
    std::string name;
    std::optional<std::unique_ptr<Expression>> initialValue;
    bool isMutable_;

public:
    // Enhanced constructor with more robust initialization
    //the errors came from tyecheck in variable node.
    VariableNode(SourceLocation loc, Type type, const std::string& name,
                 bool isMutable = true,
                 std::optional<std::unique_ptr<Expression>> initialValue = std::nullopt)
        : Expression(loc, type),
        name(name),
        initialValue(std::move(initialValue)),
        isMutable_(isMutable) {
        // Additional validation can be added here
        // if (initialValue && type.tag == TypeTag::Any) {
        //     // Try to infer type from initializer if not explicitly specified
        //     type = (*initialValue)->type;
        // }
    }

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

    // Override inferType
    TypePtr inferType(TypeSystem &typeSystem) override {
        if (initialValue) {
            // Infer type from the initializer if present
            return (*initialValue)->inferType(typeSystem);
        }
        // Otherwise, return ANY_TYPE
        return typeSystem.ANY_TYPE;
    }

    // Override typeCheckChildren to type-check the initializer
    void typeCheckChildren(TypeSystem &typeSystem) override {
        if (initialValue) {
            (*initialValue)->typeCheckChildren(typeSystem);
        }
    }
};

class AssignmentNode : public Expression {
public:
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    AssignmentNode(SourceLocation loc, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
        : Expression(loc, Type(TypeTag::Any)),
         left(std::move(left)), right(std::move(right)) {}

    // Enable move semantics
    AssignmentNode(AssignmentNode&&) = default;
    AssignmentNode& operator=(AssignmentNode&&) = default;

    // Disable copying
    AssignmentNode(const AssignmentNode&) = delete;
    AssignmentNode& operator=(const AssignmentNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Function Support
class Parameter {
public:
    std::string name;
    Type type;
    std::optional<Value> defaultValue; // From ../values.hh

    Parameter(const std::string &name, Type type, std::optional<Value> defaultValue = std::nullopt)
        : name(name), type(type), defaultValue(defaultValue) {}
};

class ReturnNode : public Statement {
public:
    std::unique_ptr<ASTNode> value;

    ReturnNode(std::unique_ptr<ASTNode> value)
        : Statement(SourceLocation(0, 0)), value(std::move(value)) {}

    // Enable move semantics
    ReturnNode(ReturnNode&&) = default;
    ReturnNode& operator=(ReturnNode&&) = default;

    // Disable copying
    ReturnNode(const ReturnNode&) = delete;
    ReturnNode& operator=(const ReturnNode&) = delete;

    void accept(ASTVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class FunctionNode : public Statement {
public:
    std::string name;
    Type returnType;
    std::vector<Parameter> parameters;
    std::vector<std::unique_ptr<Statement>> body;

    FunctionNode(SourceLocation loc, const std::string &name, Type returnType,
                 std::vector<Parameter> params, std::vector<std::unique_ptr<Statement>> body)
        : Statement(loc), name(name), returnType(returnType), parameters(std::move(params)),
        body(std::move(body)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    // Override typeCheck to check the function's return type and parameters
    // bool typeCheck(TypeSystem &typeSystem, TypePtr expectedType) override {
    //     // Check parameter types
    //     for (const auto &param : parameters) {
    //         if (!typeSystem.checkType(param.defaultValue, param.type)) {
    //             getOrCreateMetadata().addError(
    //                 NodeMetadata::ErrorDetail::Severity::Error,
    //                 "Default value for parameter " + param.name + " does not match its type"
    //                 );
    //             return false;
    //         }
    //     }

    //     // Check return type
    //     if (auto *returnNode = dynamic_cast<ReturnNode*>(body.back().get())) {
    //         TypePtr returnExprType = returnNode->inferType(typeSystem);
    //         if (!typeSystem.isCompatible(returnExprType, returnType)) {
    //             getOrCreateMetadata().addError(
    //                 NodeMetadata::ErrorDetail::Severity::Error,
    //                 "Return type mismatch: expected " + returnType->toString() + ", got " + returnExprType->toString()
    //                 );
    //             return false;
    //         }
    //     }

    //     return true;
    // }

    // Override typeCheckChildren to type-check the function body
    void typeCheckChildren(TypeSystem &typeSystem) override {
        for (auto &stmt : body) {
            stmt->typeCheckChildren(typeSystem);
        }
    }
};

// Class Support
class ClassNode : public Statement {
public:
    std::string name;
    std::optional<std::string> baseClass; // For inheritance
    std::vector<std::unique_ptr<Statement>> members; // Methods and fields

    ClassNode(SourceLocation loc, const std::string &name,
              std::optional<std::string> baseClass = std::nullopt,
              std::vector<std::unique_ptr<Statement>> members = {})
        : Statement(loc), name(name), baseClass(baseClass), members(std::move(members)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Example Nodes for Class Members
class FieldNode : public Statement {
public:
    std::string name;
    Type type;
    std::optional<Value> initialValue; // From ../values.hh

    FieldNode(SourceLocation loc, const std::string &name, Type type,
              std::optional<Value> initialValue = std::nullopt)
        : Statement(loc), name(name), type(type), initialValue(initialValue) {}

    void accept(ASTVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class MethodNode : public FunctionNode {
public:
    bool isStatic;

    MethodNode(SourceLocation loc, const std::string &name, Type returnType,
               std::vector<Parameter> params, std::vector<std::unique_ptr<Statement>> body,
               bool isStatic = false)
        : FunctionNode(loc, name, returnType, std::move(params), std::move(body)), isStatic(isStatic) {}

    void accept(ASTVisitor &visitor) override {
        // Delegate to the base FunctionNode's visit method
        visitor.visit(static_cast<FunctionNode&>(*this));
    }
};

// Module Support
class ModuleNode : public Statement {
public:
    std::string name;
    std::unique_ptr<Statement> body;

    ModuleNode(const std::string& name, std::unique_ptr<Statement> body)
        : Statement(SourceLocation(0, 0)), name(name), body(std::move(body)) {}

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

    ErrorHandlingNode(SourceLocation loc, Type type, ErrorType errType,
                      std::unique_ptr<Expression> expr,
                      std::optional<std::unique_ptr<Expression>> handler = std::nullopt)
        : Expression(loc, type), errorType(errType), expression(std::move(expr)),
        errorHandler(std::move(handler)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Pattern Matching Node
class MatchNode : public Statement {
public:
    std::unique_ptr<Expression> matchExpression;
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> matchCases;
    std::optional<std::unique_ptr<Statement>> defaultCase;

    MatchNode(SourceLocation loc, std::unique_ptr<Expression> expr,
              std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> cases,
              std::optional<std::unique_ptr<Statement>> defaultCase = std::nullopt)
        : Statement(loc), matchExpression(std::move(expr)), matchCases(std::move(cases)),
        defaultCase(std::move(defaultCase)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Stream Processing Node
class StreamProcessingNode : public Statement {
public:
    std::unique_ptr<Expression> inputStream;
    std::unique_ptr<Expression> processingFunction;
    std::unique_ptr<Expression> outputChannel;

    StreamProcessingNode(SourceLocation loc, std::unique_ptr<Expression> input,
                         std::unique_ptr<Expression> processFunc, std::unique_ptr<Expression> output)
        : Statement(loc), inputStream(std::move(input)), processingFunction(std::move(processFunc)),
        outputChannel(std::move(output)) {}

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

    AtomicNode(SourceLocation loc, Type type, Operation operation,
               std::unique_ptr<Expression> target,
               std::optional<std::unique_ptr<Expression>> value = std::nullopt)
        : Expression(loc, type), op(operation), target(std::move(target)),
        value(std::move(value)) {}

    // Enable move semantics
    AtomicNode(AtomicNode&&) = default;
    AtomicNode& operator=(AtomicNode&&) = default;

    // Disable copying
    AtomicNode(const AtomicNode&) = delete;
    AtomicNode& operator=(const AtomicNode&) = delete;

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

    ChannelNode(SourceLocation loc, ChannelOperation operation,
                std::unique_ptr<Expression> channelExpr,
                std::optional<std::unique_ptr<Expression>> channelData = std::nullopt)
        : Statement(loc), op(operation), channel(std::move(channelExpr)),
        data(std::move(channelData)) {}

    ChannelNode(ChannelNode&&) = default;
    ChannelNode& operator=(ChannelNode&&) = default;

    ChannelNode(const ChannelNode&) = delete;
    ChannelNode& operator=(const ChannelNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class LambdaNode : public Expression {
public:
    std::vector<Parameter> parameters;
    std::unique_ptr<Statement> body;

    LambdaNode(SourceLocation loc, Type type, std::vector<Parameter> params, std::unique_ptr<Statement> body)
        : Expression(loc, type), parameters(std::move(params)), body(std::move(body)) {}

    // Enable move semantics
    LambdaNode(LambdaNode&&) = default;
    LambdaNode& operator=(LambdaNode&&) = default;

    // Disable copying
    LambdaNode(const LambdaNode&) = delete;
    LambdaNode& operator=(const LambdaNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ImportNode : public Statement {
public:
    std::string moduleName;
    std::optional<std::string> alias;

    ImportNode(SourceLocation loc, const std::string &moduleName, std::optional<std::string> alias = std::nullopt)
        : Statement(loc), moduleName(moduleName), alias(std::move(alias)) {}

    // Enable move semantics
    ImportNode(ImportNode&&) = default;
    ImportNode& operator=(ImportNode&&) = default;

    // Disable copying
    ImportNode(const ImportNode&) = delete;
    ImportNode& operator=(const ImportNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// New Node for Interface
class InterfaceNode : public Statement {
public:
    std::string name;
    std::vector<std::unique_ptr<Statement>> methods;

    InterfaceNode(SourceLocation loc, const std::string &name, std::vector<std::unique_ptr<Statement>> methods)
        : Statement(loc), name(name), methods(std::move(methods)) {}

    // Enable move semantics
    InterfaceNode(InterfaceNode&&) = default;
    InterfaceNode& operator=(InterfaceNode&&) = default;

    // Disable copying
    InterfaceNode(const InterfaceNode&) = delete;
    InterfaceNode& operator=(const InterfaceNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// New Node for Mixin
class MixinNode : public Statement {
public:
    std::string name;
    std::vector<std::unique_ptr<Statement>> methods;

    MixinNode(SourceLocation loc, const std::string &name, std::vector<std::unique_ptr<Statement>> methods)
        : Statement(loc), name(name), methods(std::move(methods)) {}

    // Enable move semantics
    MixinNode(MixinNode&&) = default;
    MixinNode& operator=(MixinNode&&) = default;

    // Disable copying
    MixinNode(const MixinNode&) = delete;
    MixinNode& operator=(const MixinNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// New Node for Unsafe Block
class UnsafeNode : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> body;

    UnsafeNode(SourceLocation loc, std::vector<std::unique_ptr<Statement>> body)
        : Statement(loc), body(std::move(body)) {}

    // Enable move semantics
    UnsafeNode(UnsafeNode&&) = default;
    UnsafeNode& operator=(UnsafeNode&&) = default;

    // Disable copying
    UnsafeNode(const UnsafeNode&) = delete;
    UnsafeNode& operator=(const UnsafeNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class InterpolatedStringNode : public Expression {
public:
    std::vector<std::unique_ptr<ASTNode>> parts;

    InterpolatedStringNode(SourceLocation loc, std::vector<std::unique_ptr<ASTNode>> parts)
        : Expression(loc, Type{TypeTag::String}), parts(std::move(parts)) {}

    // Enable move semantics
    InterpolatedStringNode(InterpolatedStringNode&&) = default;
    InterpolatedStringNode& operator=(InterpolatedStringNode&&) = default;

    // Disable copying
    InterpolatedStringNode(const InterpolatedStringNode&) = delete;
    InterpolatedStringNode& operator=(const InterpolatedStringNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};



class UnaryNode : public Expression {
public:
    std::string operatorType;
    std::unique_ptr<ASTNode> operand;

    UnaryNode(Token token, std::unique_ptr<ASTNode> operand)
        : Expression(SourceLocation(token.line, token.column, token.filename), Type(TypeTag::Any)),
        operatorType(token.lexeme), operand(std::move(operand)) {}

    // Enable move semantics
    UnaryNode(UnaryNode&&) = default;
    UnaryNode& operator=(UnaryNode&&) = default;

    // Disable copying
    UnaryNode(const UnaryNode&) = delete;
    UnaryNode& operator=(const UnaryNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

// Fix CallNode implementation
class CallNode : public Expression {
public:
    std::string name;
    std::vector<std::unique_ptr<Expression>> arguments;

    CallNode(SourceLocation loc, const Token& token, const std::string& name, std::vector<std::unique_ptr<Expression>> args)
        : Expression(loc, Type(TypeTag::Any)),
        name(name), arguments(std::move(args)) {}

    CallNode(SourceLocation loc, const std::string& name, std::vector<std::unique_ptr<ASTNode>> args)
        : Expression(loc, Type(TypeTag::Any)), name(name) {
        arguments.reserve(args.size());
        for (auto& arg : args) {
            if (auto* expr = dynamic_cast<Expression*>(arg.get())) {
                arguments.push_back(std::unique_ptr<Expression>(static_cast<Expression*>(arg.release())));
            } else {
                // Clean up remaining arguments before throwing
                for (auto& a : args) {
                    if (a) a.reset();
                }
                throw std::runtime_error("Non-expression argument in function call");
            }
        }
    }

    // Enable move semantics
    CallNode(CallNode&&) = default;
    CallNode& operator=(CallNode&&) = default;

    // Disable copying
    CallNode(const CallNode&) = delete;
    CallNode& operator=(const CallNode&) = delete;

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class GroupingNode : public Expression {
public:
    std::unique_ptr<ASTNode> expression;

    GroupingNode(std::unique_ptr<ASTNode> expr)
        : Expression(SourceLocation(0, 0), Type(TypeTag::Any)), expression(std::move(expr)) {}

    // Enable move semantics
    GroupingNode(GroupingNode&&) = default;
    GroupingNode& operator=(GroupingNode&&) = default;

    // Disable copying
    GroupingNode(const GroupingNode&) = delete;
    GroupingNode& operator=(const GroupingNode&) = delete;

    void accept(ASTVisitor &visitor) override {
        visitor.visit(*this);
    }
};

class NilNode : public Expression {
public:
    NilNode(SourceLocation loc)
        : Expression(loc, Type{TypeTag::Nil}) {}

    // Enable move semantics
    NilNode(NilNode&&) = default;
    NilNode& operator=(NilNode&&) = default;

    // Disable copying
    NilNode(const NilNode&) = delete;
    NilNode& operator=(const NilNode&) = delete;

    void accept(ASTVisitor &visitor) override {
        visitor.visit(*this);
    }
};

// Utility function to help with AST node creation and error handling
template<typename NodeType, typename... Args>
std::unique_ptr<NodeType> createASTNode(Args&&... args) {
    try {
        return std::make_unique<NodeType>(std::forward<Args>(args)...);
    } catch (const std::exception& e) {
        // Log the error or handle it appropriately
        // You might want to replace this with your project's error handling mechanism
        std::cerr << "Error creating AST node: " << e.what() << std::endl;
        return nullptr;
    }
}

#endif // LUMINAR_AST_HH
