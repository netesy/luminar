#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

#include "jit.hh"
#include "../ast.hh"
#include "../token.hh"
#include "../opcodes.hh"
#include "../types.hh"
#include "../variable.hh"

#include <libgccjit.h>

JitCompiler::JitCompiler() {
    m_context = gcc_jit_context_acquire();
    if (!m_context) {
        throw std::runtime_error("Failed to acquire JIT context");
    }
    m_result = nullptr;
    m_block = nullptr;
}

JitCompiler::~JitCompiler() {
    if (m_result) {
        gcc_jit_result_release(m_result);
    }
    if (m_context) {
        gcc_jit_context_release(m_context);
    }
}

void JitCompiler::compile(const std::vector<std::unique_ptr<ASTNode>>& ast) {
    // Create a function to hold the compiled code
    gcc_jit_function *func = gcc_jit_context_new_function(
        m_context,
        NULL,
        GCC_JIT_FUNCTION_EXPORTED,
        gcc_jit_context_get_type(m_context, GCC_JIT_TYPE_VOID),
        "main",
        0,
        NULL,
        0
    );

    // Create a block to hold the code
    m_block = gcc_jit_function_new_block(func, "entry");

    for (const auto& node : ast) {
        node->accept(*this);
    }

    // End the block with a return
    gcc_jit_block_end_with_void_return(m_block, NULL);

    // Compile the function
    m_result = gcc_jit_context_compile(m_context);
    if (!m_result) {
        throw std::runtime_error("Failed to compile JIT code");
    }
}

void JitCompiler::execute() {
    if (!m_result) {
        throw std::runtime_error("No compiled code to execute");
    }
    void (*func)() = (void (*)())gcc_jit_result_get_code(m_result, "main");
    if (!func) {
        throw std::runtime_error("Failed to get function pointer");
    }
    func();
}

void JitCompiler::visit(NumberNode& node) {
    // For simplicity, we'll just print the number for now
    // In a real compiler, you would generate code to handle the number
    double value = std::stod(node.getValue());
    gcc_jit_rvalue *rvalue = gcc_jit_context_new_rvalue_from_double(
        m_context,
        gcc_jit_context_get_type(m_context, GCC_JIT_TYPE_DOUBLE),
        value
    );
    // We need a function to print a double
    gcc_jit_function *printf_func = gcc_jit_context_new_function(
        m_context,
        NULL,
        GCC_JIT_FUNCTION_IMPORTED,
        gcc_jit_context_get_type(m_context, GCC_JIT_TYPE_INT),
        "printf",
        1,
        (gcc_jit_param**)&gcc_jit_context_get_type(m_context, GCC_JIT_TYPE_CONST_CHAR_PTR),
        1
    );
    gcc_jit_rvalue *format_str = gcc_jit_context_new_string_literal(m_context, "%f\n");
    gcc_jit_rvalue *args[] = {format_str, rvalue};
    gcc_jit_block_add_eval(m_block, NULL, gcc_jit_context_new_call(m_context, NULL, printf_func, 2, args));
}

void JitCompiler::visit(StringLiteralNode& node) {
    // Similar to NumberNode, we'll print the string
    gcc_jit_function *printf_func = gcc_jit_context_new_function(
        m_context,
        NULL,
        GCC_JIT_FUNCTION_IMPORTED,
        gcc_jit_context_get_type(m_context, GCC_JIT_TYPE_INT),
        "printf",
        1,
        (gcc_jit_param**)&gcc_jit_context_get_type(m_context, GCC_JIT_TYPE_CONST_CHAR_PTR),
        1
    );
    gcc_jit_rvalue *format_str = gcc_jit_context_new_string_literal(m_context, "%s\n");
    gcc_jit_rvalue *value_str = gcc_jit_context_new_string_literal(m_context, node.getValue().c_str());
    gcc_jit_rvalue *args[] = {format_str, value_str};
    gcc_jit_block_add_eval(m_block, NULL, gcc_jit_context_new_call(m_context, NULL, printf_func, 2, args));
}

void JitCompiler::visit(BinaryNode& node) {
    // For now, we'll just visit the left and right nodes
    // A real implementation would handle the operation
    node.getLeft()->accept(*this);
    node.getRight()->accept(*this);
}

void JitCompiler::visit(PrintNode& node) {
    node.getChild()->accept(*this);
}
