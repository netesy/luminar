//opcodes.hh
#pragma once
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

enum class Opcode {
    // Arithmetic operations
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULUS,

    // Comparison operations
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

    // Fields for saving values
    int32_t intValue; // For integer values
    float floatValue; // For floating-point values
    bool boolValue;   // For boolean values
    std::string stringValue; // For string values

    // Constructor for instructions with integer value
    Instruction(Opcode op, uint32_t line, int32_t value)
        : opcode(op)
        , lineNumber(line)
        , intValue(value)
    {}

    // Constructor for instructions with floating-point value
    Instruction(Opcode op, uint32_t line, float value)
        : opcode(op)
        , lineNumber(line)
        , floatValue(value)
    {}

    // Constructor for instructions with boolean value
    Instruction(Opcode op, uint32_t line, bool value)
        : opcode(op)
        , lineNumber(line)
        , boolValue(value)
    {}

    // Constructor for instructions with string value
    Instruction(Opcode op, uint32_t line, const std::string &value)
        : opcode(op)
        , lineNumber(line)
        , stringValue(value)
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
        switch (opcode) {
        // Arithmetic operations
        case Opcode::ADD:
        case Opcode::SUBTRACT:
        case Opcode::MULTIPLY:
        case Opcode::DIVIDE:
        case Opcode::MODULUS:
            std::cout << "Value (int): " << intValue << std::endl;
            break;

            // Comparison operations
        case Opcode::EQUAL:
        case Opcode::NOT_EQUAL:
        case Opcode::LESS_THAN:
        case Opcode::LESS_THAN_OR_EQUAL:
        case Opcode::GREATER_THAN:
        case Opcode::GREATER_THAN_OR_EQUAL:
            std::cout << "Value (bool): " << std::boolalpha << boolValue << std::endl;
            break;

            // Logical operations
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::NOT:
            std::cout << "Value (bool): " << std::boolalpha << boolValue << std::endl;
            break;

            // Control flow operations
        case Opcode::JUMP:
        case Opcode::JUMP_IF_TRUE:
        case Opcode::JUMP_IF_FALSE:
            std::cout << "Value (int): " << intValue << std::endl;
            break;

            // Variable operations
        case Opcode::DECLARE_VARIABLE:
        case Opcode::LOAD_VARIABLE:
        case Opcode::STORE_VARIABLE:
            std::cout << "Value (string): " << stringValue << std::endl;
            break;

            // Other operations
        case Opcode::NOP:
        case Opcode::HALT:
        case Opcode::PRINT:
            std::cout << "No value" << std::endl;
            break;

            // Function definition and invocation
        case Opcode::DEFINE_FUNCTION:
        case Opcode::INVOKE_FUNCTION:
        case Opcode::RETURN_VALUE:
            std::cout << "Value (string): " << stringValue << std::endl;
            break;

            // Loop operations
        case Opcode::FOR_LOOP:
        case Opcode::WHILE_LOOP:
            std::cout << "Value (int): " << intValue << std::endl;
            break;

            // Error handling operations
        case Opcode::ATTEMPT:
        case Opcode::HANDLE:
            std::cout << "No value" << std::endl;
            break;

            // Class operations
        case Opcode::DEFINE_CLASS:
        case Opcode::CREATE_OBJECT:
        case Opcode::METHOD_CALL:
            std::cout << "Value (string): " << stringValue << std::endl;
            break;

            // File I/O operations
        case Opcode::OPEN_FILE:
        case Opcode::WRITE_FILE:
        case Opcode::CLOSE_FILE:
            std::cout << "Value (string): " << stringValue << std::endl;
            break;

            // Concurrency operations
        case Opcode::PARALLEL:
        case Opcode::CONCURRENT:
        case Opcode::ASYNC:
            std::cout << "No value" << std::endl;
            break;

            // Generics operations
        case Opcode::GENERIC_FUNCTION:
        case Opcode::GENERIC_TYPE:
            std::cout << "Value (string): " << stringValue << std::endl;
            break;

            // Pattern matching operations
        case Opcode::PATTERN_MATCH:
            std::cout << "No value" << std::endl;
            break;

            // Additional operations for saving, retrieving values, and string manipulation
        case Opcode::LOAD_VALUE:
        case Opcode::STORE_VALUE:
        case Opcode::LOAD_STR:
        case Opcode::STORE_STR:
        case Opcode::CONCATENATE_STR:
            std::cout << "Value (string): " << stringValue  << intValue << std::endl;
            break;

            // Unrecognized opcode
        default:
            std::cout << "Unrecognized opcode" << std::endl;
            break;
        }
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

            // Unrecognized opcode
        default:
            return "UNKNOWN";
        }
    }
};
