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

// #pragma once

// #include <cstdint>

// /**
//  * Bytecode operation codes organized by functional category
//  * Optimized for libgccjit integration
//  */
// enum class Opcode : uint8_t {
//     // Core control flow (0x00-0x0F)
//     NOP             = 0x00,  // No operation
//     HALT            = 0x01,  // Halt execution
//     JUMP            = 0x02,  // Unconditional jump
//     JUMP_IF_TRUE    = 0x03,  // Jump if condition is true
//     JUMP_IF_FALSE   = 0x04,  // Jump if condition is false
//     RETURN          = 0x05,  // Return from function without value
//     RETURN_VALUE    = 0x06,  // Return value from function

//     // Function operations (0x10-0x1F)
//     DEFINE_FUNCTION    = 0x10,  // Define function
//     INVOKE_FUNCTION    = 0x11,  // Call function
//     PUSH_ARGS          = 0x12,  // Push arguments for function call
//     CREATE_PARAM_FRAME = 0x13,  // Create parameter frame
//     STORE_PARAM        = 0x14,  // Store parameter in frame
//     LOAD_PARAM         = 0x15,  // Load parameter from frame
//     POP_PARAM_FRAME    = 0x16,  // Remove parameter frame
//     GENERIC_FUNCTION   = 0x17,  // Generic function definition

//     // Variable operations (0x20-0x2F)
//     DECLARE_VARIABLE = 0x20,  // Declare variable
//     LOAD_VARIABLE    = 0x21,  // Load variable
//     STORE_VARIABLE   = 0x22,  // Store variable
//     LOAD_CONST       = 0x23,  // Load numerical constant
//     LOAD_VALUE       = 0x24,  // Load value from memory
//     STORE_VALUE      = 0x25,  // Store value to memory
//     LOAD_STR         = 0x26,  // Load string from memory
//     STORE_STR        = 0x27,  // Store string to memory

//     // Arithmetic operations (0x30-0x3F)
//     ADD      = 0x30,  // Addition
//     SUBTRACT = 0x31,  // Subtraction
//     MULTIPLY = 0x32,  // Multiplication
//     DIVIDE   = 0x33,  // Division
//     MODULUS  = 0x34,  // Modulus
//     NEGATE   = 0x35,  // Numeric negation

//     // Assignment operations (0x40-0x4F)
//     ADD_ASSIGN = 0x40,  // Add and assign
//     SUB_ASSIGN = 0x41,  // Subtract and assign

//     // Comparison operations (0x50-0x5F)
//     EQUAL                 = 0x50,  // Equal comparison
//     NOT_EQUAL             = 0x51,  // Not equal comparison
//     LESS_THAN             = 0x52,  // Less than comparison
//     LESS_THAN_OR_EQUAL    = 0x53,  // Less than or equal comparison
//     GREATER_THAN          = 0x54,  // Greater than comparison
//     GREATER_THAN_OR_EQUAL = 0x55,  // Greater than or equal comparison
//     COMPARE               = 0x56,  // Generic comparison

//     // Logical operations (0x60-0x6F)
//     AND     = 0x60,  // Logical AND
//     OR      = 0x61,  // Logical OR
//     NOT     = 0x62,  // Logical NOT
//     BOOLEAN = 0x63,  // Convert to boolean

//     // Loop operations (0x70-0x7F)
//     FOR_LOOP    = 0x70,  // For loop
//     WHILE_LOOP  = 0x71,  // While loop
//     MAKE_RANGE  = 0x72,  // Create range for iteration

//     // String operations (0x80-0x8F)
//     CONCATENATE_STR    = 0x80,  // Concatenate strings
//     INTERPOLATE_STRING = 0x81,  // Interpolate strings
//     PRINT              = 0x82,  // Print output

//     // Error handling (0x90-0x9F)
//     ATTEMPT       = 0x90,  // Try block
//     HANDLE        = 0x91,  // Catch block
//     ERROR_STRATEGY = 0x92,  // Error handling strategy
//     MATCH_TYPE    = 0x93,  // Type checking
//     PATTERN_MATCH = 0x94,  // Pattern matching

//     // Object-oriented programming (0xA0-0xAF)
//     DEFINE_CLASS       = 0xA0,  // Define class
//     CREATE_OBJECT      = 0xA1,  // Create object instance
//     INVOKE_CONSTRUCTOR = 0xA2,  // Call constructor
//     DEFINE_METHOD      = 0xA3,  // Define method
//     INVOKE_METHOD      = 0xA4,  // Call method
//     METHOD_CALL        = 0xA5,  // Method call shorthand
//     LOAD_PROPERTY      = 0xA6,  // Get property
//     STORE_PROPERTY     = 0xA7,  // Set property
//     SUPER_CALL         = 0xA8,  // Call parent method
//     GENERIC_TYPE       = 0xA9,  // Generic type definition

//     // File operations (0xB0-0xBF)
//     OPEN_FILE  = 0xB0,  // Open file
//     WRITE_FILE = 0xB1,  // Write to file
//     CLOSE_FILE = 0xB2,  // Close file

//     // Concurrency operations (0xC0-0xCF)
//     PARALLEL_END     = 0xC0,  // End parallel block
//     TASK_PARALLEL    = 0xC1,  // Parallel task
//     PARALLEL_CORES   = 0xC2,  // Set parallel cores
//     CONCURRENT_END   = 0xC3,  // End concurrent block
//     TASK_CONCURRENT  = 0xC4,  // Concurrent task
//     CHANNEL_DEFINE   = 0xC5,  // Define communication channel
//     INPUT_SOURCE     = 0xC6,  // Set input source
//     OUTPUT_CHANNEL   = 0xC7,  // Set output channel
//     WORKER_FUNCTION  = 0xC8,  // Define worker function

//     // Memory management (0xD0-0xDF)
//     ALLOC            = 0xD0,  // Allocate memory
//     DEALLOC          = 0xD1,  // Deallocate memory
//     RESIZE           = 0xD2,  // Resize allocation
//     COPY             = 0xD3,  // Copy memory
//     SET              = 0xD4,  // Set memory
//     MOVE             = 0xD5,  // Move memory
//     ALLOCATE_ZEROED  = 0xD6,  // Allocate zeroed memory
//     SET_UNSAFE_MODE  = 0xD7   // Toggle unsafe memory operations
// };
