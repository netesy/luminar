#pragma once

#include "../ast.hh"
#include <libgccjit++.h>
#include <vector>

class JitCompiler : public JitVisitor {
public:
    JitCompiler(std::vector<std::unique_ptr<ASTNode>>& ast);
    void compile();

    void visit(class NumberNode &node, void* data) override;
    void visit(class StringLiteralNode &node, void* data) override;
    void visit(class BinaryNode &node, void* data) override;
    void visit(class ConditionalNode &node, void* data) override;
    void visit(class BlockNode &node, void* data) override;
    void visit(class WhileNode &node, void* data) override;
    void visit(class ForNode &node, void* data) override;
    void visit(class RangeNode &node, void* data) override;
    void visit(class ListNode &node, void* data) override;
    void visit(class DictNode &node, void* data) override;
    void visit(class ParallelNode &node, void* data) override;
    void visit(class VariableNode &node, void* data) override;
    void visit(class AssignmentNode &node, void* data) override;
    void visit(class ConcurrentNode &node, void* data) override;
    void visit(class FunctionNode &node, void* data) override;
    void visit(class ClassNode &node, void* data) override;
    void visit(class ModuleNode &node, void* data) override;
    void visit(class ErrorHandlingNode &node, void* data) override;
    void visit(class MatchNode &node, void* data) override;
    void visit(class StreamProcessingNode &node, void* data) override;
    void visit(class AtomicNode &node, void* data) override;
    void visit(class ChannelNode &node, void* data) override;
    void visit(class FieldNode &node, void* data) override;
    void visit(class LambdaNode &node, void* data) override;
    void visit(class ImportNode &node, void* data) override;
    void visit(class InterfaceNode &node, void* data) override;
    void visit(class MixinNode &node, void* data) override;
    void visit(class UnsafeNode &node, void* data) override;
    void visit(class InterpolatedStringNode &node, void* data) override;
    void visit(class BooleanNode &node, void* data) override;
    void visit(class UnaryNode &node, void* data) override;
    void visit(class CallNode &node, void* data) override;
    void visit(class GroupingNode &node, void* data) override;
    void visit(class ReturnNode &node, void* data) override;
    void visit(class NilNode &node, void* data) override;
    void visit(class PrintNode &node, void* data) override;

private:
    std::vector<std::unique_ptr<ASTNode>>& ast;
    gcc_jit_context *ctxt;
    gcc_jit_result *result;
    gcc_jit_function *current_function;
    gcc_jit_rvalue *last_rvalue;
};
