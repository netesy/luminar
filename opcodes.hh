//opcodes.hh
#pragma once
#include <any>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

enum Opcode {
    // Arithmetic operations
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULUS,

    // Comparison operations
    ADD_ASSIGN,
    SUB_ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL,

    // Logical operations
    AND,
    OR,
    NOT,

    // Control flow operations
    JUMP,
    JUMP_IF_TRUE,
    JUMP_IF_FALSE,
    RETURN,

    // Variable operations
    DECLARE_VARIABLE,
    LOAD_VARIABLE,
    STORE_VARIABLE,

    // Other operations
    NOP,   // No operation
    HALT,  // Halt execution
    PRINT, // Print

    // Function definition and invocation
    DEFINE_FUNCTION,
    INVOKE_FUNCTION,
    RETURN_VALUE,

    // Loop operations
    FOR_LOOP,
    WHILE_LOOP,

    // Error handling operations
    ATTEMPT,
    HANDLE,

    // Class operations
    DEFINE_CLASS,
    CREATE_OBJECT,
    METHOD_CALL,

    // File I/O operations
    OPEN_FILE,
    WRITE_FILE,
    CLOSE_FILE,

    // Concurrency operations
    PARALLEL,
    CONCURRENT,
    ASYNC,

    // Generics operations
    GENERIC_FUNCTION,
    GENERIC_TYPE,

    // Pattern matching operations (potentially included)
    PATTERN_MATCH,

    // Additional operations for saving, retrieving values, and string manipulation
    LOAD_CONST,     //load numerical const to and from memory
    LOAD_VALUE,     // Load value from memory
    STORE_VALUE,    // Store value to memory
    LOAD_STR,       // Load strings from memory
    STORE_STR,      // Store strings to memory
    CONCATENATE_STR // Concatenate strings
};

// Define a struct to represent bytecode instructions
struct Instruction
{
    Opcode opcode;
    uint32_t lineNumber; // Line number in the source code
    // Additional fields for operands, labels, etc.
    // Add any other metadata needed for debugging or bytecode generation

    // Use std::variant to hold any type of value
    std::variant<int32_t, double, bool, std::string> value;

    // Constructor for instructions with string value
    Instruction(Opcode op,
                uint32_t line,
                const std::variant<int32_t, double, bool, std::string> &value)
        : opcode(op)
        , lineNumber(line)
        , value(value)
    {}

    // Constructor for instructions without value
    Instruction(Opcode op, uint32_t line)
        : opcode(op)
        , lineNumber(line)
    {}

    // Debug function to display content of the struct
    void debug() const
    {
        std::cout << "Opcode: " << opcodeToString(opcode) << std::endl;
        std::cout << "Line Number: " << lineNumber << std::endl;
        std::visit(
            [&](auto const &val) {
                std::cout << "Value: ";
                if (std::is_same_v<decltype(val), std::string>) {
                    std::cout << val << std::endl;
                } else {
                    if constexpr (std::is_integral_v<decltype(val)>) {
                        std::cout << "int: " << val << std::endl;
                    } else if (std::is_floating_point_v<decltype(val)>) {
                        std::cout << "float: " << val << std::endl;
                    } else if (std::is_same_v<decltype(val), bool>) {
                        std::cout << "bool: " << std::boolalpha << val << std::endl;
                    } else {
                        std::cout << val << std::endl;
                    }
                }
            },
            value);
    }

    std::string opcodeToString(Opcode op) const
    {
        switch (op) {
        // Arithmetic operations
        case Opcode::ADD:
            return "ADD";
        case Opcode::SUBTRACT:
            return "SUBTRACT";
        case Opcode::MULTIPLY:
            return "MULTIPLY";
        case Opcode::DIVIDE:
            return "DIVIDE";
        case Opcode::MODULUS:
            return "MODULUS";

            // Comparison operations
        case Opcode::EQUAL:
            return "EQUAL";
        case Opcode::NOT_EQUAL:
            return "NOT_EQUAL";
        case Opcode::LESS_THAN:
            return "LESS_THAN";
        case Opcode::LESS_THAN_OR_EQUAL:
            return "LESS_THAN_OR_EQUAL";
        case Opcode::GREATER_THAN:
            return "GREATER_THAN";
        case Opcode::GREATER_THAN_OR_EQUAL:
            return "GREATER_THAN_OR_EQUAL";

            // Logical operations
        case Opcode::AND:
            return "AND";
        case Opcode::OR:
            return "OR";
        case Opcode::NOT:
            return "NOT";

            // Control flow operations
        case Opcode::JUMP:
            return "JUMP";
        case Opcode::JUMP_IF_TRUE:
            return "JUMP_IF_TRUE";
        case Opcode::JUMP_IF_FALSE:
            return "JUMP_IF_FALSE";
        case Opcode::RETURN:
            return "RETURN";

            // Variable operations
        case Opcode::DECLARE_VARIABLE:
            return "DECLARE_VARIABLE";
        case Opcode::LOAD_VARIABLE:
            return "LOAD_VARIABLE";
        case Opcode::STORE_VARIABLE:
            return "STORE_VARIABLE";

            // Other operations
        case Opcode::NOP:
            return "NOP";
        case Opcode::HALT:
            return "HALT";
        case Opcode::PRINT:
            return "PRINT";

            // Function definition and invocation
        case Opcode::DEFINE_FUNCTION:
            return "DEFINE_FUNCTION";
        case Opcode::INVOKE_FUNCTION:
            return "INVOKE_FUNCTION";
        case Opcode::RETURN_VALUE:
            return "RETURN_VALUE";

            // Loop operations
        case Opcode::FOR_LOOP:
            return "FOR_LOOP";
        case Opcode::WHILE_LOOP:
            return "WHILE_LOOP";

            // Error handling operations
        case Opcode::ATTEMPT:
            return "ATTEMPT";
        case Opcode::HANDLE:
            return "HANDLE";

            // Class operations
        case Opcode::DEFINE_CLASS:
            return "DEFINE_CLASS";
        case Opcode::CREATE_OBJECT:
            return "CREATE_OBJECT";
        case Opcode::METHOD_CALL:
            return "METHOD_CALL";

            // File I/O operations
        case Opcode::OPEN_FILE:
            return "OPEN_FILE";
        case Opcode::WRITE_FILE:
            return "WRITE_FILE";
        case Opcode::CLOSE_FILE:
            return "CLOSE_FILE";

            // Concurrency operations
        case Opcode::PARALLEL:
            return "PARALLEL";
        case Opcode::CONCURRENT:
            return "CONCURRENT";
        case Opcode::ASYNC:
            return "ASYNC";

            // Generics operations
        case Opcode::GENERIC_FUNCTION:
            return "GENERIC_FUNCTION";
        case Opcode::GENERIC_TYPE:
            return "GENERIC_TYPE";

            // Pattern matching operations
        case Opcode::PATTERN_MATCH:
            return "PATTERN_MATCH";

            // Additional operations for saving, retrieving values, and string manipulation
        case Opcode::LOAD_VALUE:
            return "LOAD_VALUE";
        case Opcode::STORE_VALUE:
            return "STORE_VALUE";
        case Opcode::LOAD_STR:
            return "LOAD_STR";
        case Opcode::STORE_STR:
            return "STORE_STR";
        case Opcode::CONCATENATE_STR:
            return "CONCATENATE_STR";
        case Opcode::LOAD_CONST:
            return "LOAD_CONST";

            // Unrecognized opcode
        default:
            return "UNKNOWN";
        }
    }
};
