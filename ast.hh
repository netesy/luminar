#pragma once

#include <string>
#include <vector>
#ifndef AST_HH
#define AST_HH

class ASTNode
{
public:
    virtual ~ASTNode() {}

    // Add virtual methods for node-specific operations (e.g., evaluation, code generation)
};

class EnumNode : public ASTNode
{
public:
    std::string name;
    std::vector<std::string> variants;

    EnumNode(const std::string &name, std::vector<std::string> variants)
        : name(name)
        , variants(variants)
    {}

    // ... Add methods for accessing and manipulating variants (optional)
};

class PatternNode : public ASTNode
{
public:
    // Type of the pattern (e.g., VariableNode, ListExprNode, etc.)
    ASTNode *type;

    // Value to match against (optional for literals)
    ASTNode *value;

    // Name of the variable to bind captured values (optional)
    std::string name;

    PatternNode(ASTNode *type, ASTNode *value = nullptr, std::string name = "")
        : type(type)
        , value(value)
        , name(name)
    {}
};

class PatternMatchNode : public ASTNode
{
public:
    // The expression to be matched against
    ASTNode *expr;

    // A list of patterns (PatternNode) to match against
    std::vector<PatternNode *> patterns;

    // Optional default branch if no pattern matches (ASTNode)
    ASTNode *defaultBranch;

    PatternMatchNode(ASTNode *expr,
                     std::vector<PatternNode *> patterns,
                     ASTNode *defaultBranch = nullptr)
        : expr(expr)
        , patterns(patterns)
        , defaultBranch(defaultBranch)
    {}
};

// Expressions
class NumberNode : public ASTNode
{
public:
    int value;
    NumberNode(int value)
        : value(value)
    {}
};

class UnaryExprNode : public ASTNode
{
public:
    char op;
    ASTNode *expr;
    UnaryExprNode(char op, ASTNode *expr)
        : op(op)
        , expr(expr)
    {}
};

class BinaryExprNode : public ASTNode
{
public:
    char op;
    ASTNode *left;
    ASTNode *right;
    BinaryExprNode(char op, ASTNode *left, ASTNode *right)
        : op(op)
        , left(left)
        , right(right)
    {}
};

class VariableNode : public ASTNode
{
public:
    std::string name;
    std::string type;         // Optional for type information
    std::string scope;        // Optional for scope tracking
    bool mut = true;          // Optional for mutability
    ASTNode *value = nullptr; // Optional for value storage

    VariableNode(const std::string &name,
                 const std::string &type = "",
                 const std::string &scope = "",
                 bool mut = true,
                 ASTNode *value = nullptr)
        : name(name)
        , type(type)
        , scope(scope)
        , mut(mut)
        , value(value)
    {}
};

class CallExprNode : public ASTNode
{
public:
    std::string name;
    std::vector<ASTNode *> args;
    CallExprNode(const std::string &name, std::vector<ASTNode *> args)
        : name(name)
        , args(args)
    {}
};

class ArrayExprNode : public ASTNode
{
public:
    std::string name; // Optional array name
    std::vector<ASTNode *> indices;
    ArrayExprNode(const std::string &name, std::vector<ASTNode *> indices)
        : name(name)
        , indices(indices)
    {}
};

class StructExprNode : public ASTNode
{
public:
    std::string name; // Optional struct name
    std::string member;
    StructExprNode(const std::string &name, const std::string &member)
        : name(name)
        , member(member)
    {}
};

class LogicalExprNode : public ASTNode
{
public:
    char op;
    ASTNode *left;
    ASTNode *right;
    LogicalExprNode(char op, ASTNode *left, ASTNode *right)
        : op(op)
        , left(left)
        , right(right)
    {}
};

class ConditionalExprNode : public ASTNode
{
public:
    ASTNode *cond;
    ASTNode *thenBranch;
    ASTNode *elseBranch;
    ConditionalExprNode(ASTNode *cond, ASTNode *thenBranch, ASTNode *elseBranch)
        : cond(cond)
        , thenBranch(thenBranch)
        , elseBranch(elseBranch)
    {}
};

// Statements
class AssignmentStmtNode : public ASTNode
{
public:
    VariableNode *var;
    ASTNode *expr;
    AssignmentStmtNode(VariableNode *var, ASTNode *expr)
        : var(var)
        , expr(expr)
    {}
};

class BlockStmtNode : public ASTNode
{
public:
    std::vector<ASTNode *> statements;
    BlockStmtNode(std::vector<ASTNode *> statements)
        : statements(statements)
    {}
};

class IfStmtNode : public ASTNode
{
public:
    ASTNode *cond;
    ASTNode *thenBranch;
    ASTNode *elseBranch;
    IfStmtNode(ASTNode *cond, ASTNode *thenBranch, ASTNode *elseBranch)
        : cond(cond)
        , thenBranch(thenBranch)
        , elseBranch(elseBranch)
    {}
};

class WhileStmtNode : public ASTNode
{
public:
    ASTNode *cond;
    ASTNode *body;
    WhileStmtNode(ASTNode *cond, ASTNode *body)
        : cond(cond)
        , body(body)
    {}
};

class ReturnStmtNode : public ASTNode
{
public:
    ASTNode *expr; // Optional expression to return
    ReturnStmtNode(ASTNode *expr = nullptr)
        : expr(expr)
    {}
};

// Other constructs
class StringLiteralNode : public ASTNode
{
public:
    std::string value;
    StringLiteralNode(const std::string &value)
        : value(value)
    {}
};

class BoolNode : public ASTNode
{
public:
    bool value;
    BoolNode(bool value)
        : value(value)
    {}
};

class ListExprNode : public ASTNode
{
public:
    std::vector<ASTNode *> elements;
    ListExprNode(std::vector<ASTNode *> elements)
        : elements(elements)
    {}
};

class DictExprNode : public ASTNode
{
public:
    std::vector<std::pair<ASTNode *, ASTNode *>> entries;
    DictExprNode(std::vector<std::pair<ASTNode *, ASTNode *>> entries)
        : entries(entries)
    {}
};

class RangeLiteralNode : public ASTNode
{
public:
    int endValue;
    RangeLiteralNode(int endValue)
        : endValue(endValue)
    {}
};

class ForInStmtNode : public ASTNode
{
public:
    VariableNode *var;
    ASTNode *iterator; // Could be RangeLiteralNode, ListExprNode, DictExprNode, etc.
    BlockStmtNode *body;
    ForInStmtNode(VariableNode *var, ASTNode *iterator, BlockStmtNode *body)
        : var(var)
        , iterator(iterator)
        , body(body)
    {}
};

class CatchStmtNode : public ASTNode
{
public:
    EnumNode *exceptionType;
    VariableNode *exceptionVar;
    BlockStmtNode *block;
    CatchStmtNode(EnumNode *exceptionType, VariableNode *exceptionVar, BlockStmtNode *block)
        : exceptionType(exceptionType)
        , exceptionVar(exceptionVar)
        , block(block)
    {}
};

class AttemptStmtNode : public ASTNode
{
public:
    BlockStmtNode *tryBlock;
    std::vector<CatchStmtNode *> catchBlocks;
    AttemptStmtNode(BlockStmtNode *tryBlock, std::vector<CatchStmtNode *> catchBlocks)
        : tryBlock(tryBlock)
        , catchBlocks(catchBlocks)
    {}
};

class ImportStmtNode : public ASTNode
{
public:
    StringLiteralNode *moduleName;
    ImportStmtNode(StringLiteralNode *moduleName)
        : moduleName(moduleName)
    {}
};

class ParallelStmtNode : public ASTNode
{
public:
    BlockStmtNode *body;
    // ... Add other fields for parallel execution details (optional)
    ParallelStmtNode(BlockStmtNode *body)
        : body(body)
    {}
};

class ConcurrencyStmtNode : public ASTNode
{
public:
    BlockStmtNode *body;
    ConcurrencyStmtNode(BlockStmtNode *body)
        : body(body)
    {}
};

class FunctionNode : public ASTNode
{
public:
    std::string name;
    std::vector<VariableNode *> parameters;         // Main parameters
    bool hasOptionalParameters;                     // Flag for optional parameters
    std::vector<VariableNode *> optionalParameters; // Optional parameters
    std::string return_type;
    BlockStmtNode *body;

    FunctionNode(const std::string &name,
                 std::vector<VariableNode *> parameters,
                 bool hasOptionalParameters,
                 std::vector<VariableNode *> optionalParameters,
                 const std::string &return_type,
                 BlockStmtNode *body)
        : name(name)
        , parameters(parameters)
        , hasOptionalParameters(hasOptionalParameters)
        , optionalParameters(optionalParameters)
        , return_type(return_type)
        , body(body)
    {}
};

class ConstructorNode : public ASTNode
{
public:
    std::string name; // Same as the class name
    std::vector<VariableNode *> parameters;
    BlockStmtNode *body;
    ConstructorNode(const std::string &name,
                    std::vector<VariableNode *> parameters,
                    BlockStmtNode *body)
        : name(name)
        , parameters(parameters)
        , body(body)
    {}
};

class ClassNode : public ASTNode
{
public:
    std::string name;
    std::vector<VariableNode *> fields;
    std::vector<FunctionNode *> methods;
    ConstructorNode *constructor; // Optional constructor
    ClassNode(const std::string &name,
              std::vector<VariableNode *> fields,
              std::vector<FunctionNode *> methods,
              ConstructorNode *constructor = nullptr)
        : name(name)
        , fields(fields)
        , methods(methods)
        , constructor(constructor)
    {}
};

#endif
