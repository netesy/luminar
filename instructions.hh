//instructions.hh
#pragma once
#include "opcodes.hh"
#include "types.hh"
#include <any>
#include <cstdint>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

// Define a struct to represent bytecode instructions
struct Instruction
{
    Opcode opcode;
    uint32_t lineNumber; // Line number in the source code
    // Additional fields for operands, labels, etc.
    // Add any other metadata needed for debugging or bytecode generation

    // Use Value from type system to hold any type of values
    ValuePtr value;

    // Constructor for instructions with string value
    Instruction(Opcode op, uint32_t line, ValuePtr &val)
        : opcode(op)
        , lineNumber(line)
        , value(val)
    {}

    // Constructor for instructions without value
    Instruction(Opcode op, uint32_t line)
        : opcode(op)
        , lineNumber(line)
        , value(nullptr)
    {}

    // Debug function to display content of the struct
    void debug() const
    {
        std::cout << "Opcode: " << opcodeToString(opcode) << std::endl;
        std::cout << "Line Number: " << lineNumber << std::endl;
        if (value) {
            std::cout << "Value: ";
            std::visit(
                [](const auto &v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::monostate>)
                        std::cout << "None" << std::endl;
                    else if constexpr (std::is_same_v<T, bool>)
                        std::cout << "bool: " << std::boolalpha << v << std::endl;
                    else if constexpr (std::is_integral_v<T>)
                        std::cout << "int: " << v << std::endl;
                    else if constexpr (std::is_floating_point_v<T>)
                        std::cout << "float: " << v << std::endl;
                    else if constexpr (std::is_same_v<T, std::string>)
                        std::cout << "string: " << v << std::endl;
                    else if constexpr (std::is_same_v<T, ListValue>)
                        std::cout << "ListValue: " << v << std::endl;
                    else if constexpr (std::is_same_v<T, DictValue>)
                        std::cout << "DictValue: " << v << std::endl;
                    else if constexpr (std::is_same_v<T, SumValue>)
                        std::cout << "SumValue: " << v << std::endl;
                    else if constexpr (std::is_same_v<T, UserDefinedValue>)
                        std::cout << "UserDefinedValue: " << v << std::endl;
                    else
                        std::cout << "complex type" << std::endl;
                },
                value->data);
        } else {
            std::cout << "Value: None" << std::endl;
        }
        //        if (value) {
        //            std::cout << "Value: ";
        //            switch (value->type->tag) {
        //            case TypeTag::Bool:
        //                std::cout << "bool: " << std::boolalpha << std::get<bool>(value->data) << std::endl;
        //                break;
        //            case TypeTag::Int:
        //            case TypeTag::Int32:
        //                std::cout << "int: " << std::get<int32_t>(value->data) << std::endl;
        //                break;
        //            case TypeTag::Int8:
        //                std::cout << "int8: " << static_cast<int>(std::get<int8_t>(value->data))
        //                          << std::endl;
        //                break;
        //            case TypeTag::Int16:
        //                std::cout << "int16: " << std::get<int16_t>(value->data) << std::endl;
        //                break;
        //            case TypeTag::Int64:
        //                std::cout << "int64: " << std::get<int64_t>(value->data) << std::endl;
        //                break;
        //            case TypeTag::UInt:
        //            case TypeTag::UInt32:
        //                std::cout << "uint: " << std::get<uint32_t>(value->data) << std::endl;
        //                break;
        //            case TypeTag::UInt8:
        //                std::cout << "uint8: " << static_cast<unsigned int>(std::get<uint8_t>(value->data))
        //                          << std::endl;
        //                break;
        //            case TypeTag::UInt16:
        //                std::cout << "uint16: " << std::get<uint16_t>(value->data) << std::endl;
        //                break;
        //            case TypeTag::UInt64:
        //                std::cout << "uint64: " << std::get<uint64_t>(value->data) << std::endl;
        //                break;
        //            case TypeTag::Float32:
        //                std::cout << "float32: " << std::get<float>(value->data) << std::endl;
        //                break;
        //            case TypeTag::Float64:
        //                std::cout << "float64: " << std::get<double>(value->data) << std::endl;
        //                break;
        //            case TypeTag::String:
        //                std::cout << "string: " << std::get<std::string>(value->data) << std::endl;
        //                break;
        //            default:
        //                std::cout << "complex type" << std::endl;
        //                break;
        //            }
        //        } else {
        //            std::cout << "Value: None" << std::endl;
        //        }
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
        case Opcode::NEGATE:
            return "NEGATE";

            // Control flow operations
        case Opcode::JUMP:
            return "JUMP";
        case Opcode::JUMP_IF_TRUE:
            return "JUMP_IF_TRUE";
        case Opcode::JUMP_IF_FALSE:
            return "JUMP_IF_FALSE";
        case Opcode::RETURN:
            return "RETURN";
        case Opcode::BOOLEAN:
            return "BOOLEAN";

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
