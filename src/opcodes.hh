//opcodes.hh
#pragma once

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
    NEGATE,
    BOOLEAN,

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
    PUSH_ARGS,
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
    LOAD_PROPERTY,
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
    LOAD_CONST,         //load numerical const to and from memory
    LOAD_VALUE,         // Load value from memory
    STORE_VALUE,        // Store value to memory
    LOAD_STR,           // Load strings from memory
    STORE_STR,          // Store strings to memory
    CONCATENATE_STR,    // Concatenate strings
    INTERPOLATE_STRING, //Interpolate strings

    //Memory Management
    ALLOC,
    DEALLOC,
    RESIZE,
    COPY,
    SET,
    COMPARE,
    MOVE,
    ALLOCATE_ZEROED,
    SET_UNSAFE_MODE
};
