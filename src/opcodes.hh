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
        // Parameter handling
    CREATE_PARAM_FRAME, // Create a new parameter frame
    STORE_PARAM,        // Store a parameter in the current frame
    LOAD_PARAM,         // Load a parameter from the current frame
    POP_PARAM_FRAME,    // Remove the current parameter frame

    // Loop operations
    FOR_LOOP,
    WHILE_LOOP,
    MAKE_RANGE,
    MATCH_TYPE, // check if the match type matchs its value

    // Error handling operations
    ATTEMPT,
    HANDLE,

    // Class operations
    // Class-related
    DEFINE_CLASS,        // Create class definition with metadata
    CREATE_OBJECT,       // Instantiate new object of a class
    INVOKE_CONSTRUCTOR,  // Call class constructor

    // Method-related
    DEFINE_METHOD,       // Define method in class
    INVOKE_METHOD,       // Call method on object
    METHOD_CALL,        // Shorthand for method invocation

    // Property access
    LOAD_PROPERTY,      // Get property value
    STORE_PROPERTY,     // Set property value

    // Inheritance
    SUPER_CALL,        // Call method on parent class

    // File I/O operations
    OPEN_FILE,
    WRITE_FILE,
    CLOSE_FILE,

    // Parallel-related opcodes
    PARALLEL_END,
    TASK_PARALLEL,
    PARALLEL_CORES,
    CHANNEL_DEFINE,
    ERROR_STRATEGY,

    // Concurrent-related opcodes
    CONCURRENT_END,
    TASK_CONCURRENT,
    INPUT_SOURCE,
    OUTPUT_CHANNEL,
    WORKER_FUNCTION,

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
