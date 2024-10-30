//// syntax.hpp
//#pragma once

//#include "opcodes.hh"
//#include "scanner.hh"

//class Syntax
//{
//public:
//        void parseFunctionDeclaration(Scanner &scanner);
//        //    void parseForRange(Scanner &scanner);
//        void parseForLoop(Scanner &scanner);
//        void parseWhileLoop(Scanner &scanner);
//        void parseConditional(Scanner &scanner);
//        void parseClassDeclaration(Scanner &scanner);
//        void parseVariableDeclaration(Scanner &scanner);
//        void parseAssignment(Scanner &scanner);
//        void parseExpression(Scanner &scanner);
//        void parseAttempt(Scanner &scanner);
//        void parseString(Scanner &scanner);
//        void parseConcurrent(Scanner &scanner);
//        void parseParallel(Scanner &scanner);
//        void parseAwait(Scanner &scanner);
//        void parseAsync(Scanner &scanner);
//        //    void parseBlock(Scanner &scanner);
//        void ternary(Scanner &scanner);
//        void logicalOr(Scanner &scanner);
//        void logicalAnd(Scanner &scanner);
//        void equality(Scanner &scanner);
//        void comparison(Scanner &scanner);
//        void addition(Scanner &scanner);
//        void subtraction(Scanner &scanner);
//        void multiplication(Scanner &scanner);
//        void division(Scanner &scanner);
//        void modulus(Scanner &scanner);
//        void unary(Scanner &scanner);
//        void primary(Scanner &scanner);
//        void parseIdentifier(Scanner &scanner);
//        void parseType(Scanner &scanner);
//        void parseArguments(Scanner &scanner);
//        void parsePatternMatching(Scanner &scanner);
//        void parseMatchCase(Scanner &scanner);

//        void parsePrintStatement(Scanner &scanner);

//        void parseReturnStatement(Scanner &scanner);

//private:
//        void emit(Opcode op, uint32_t lineNumber, int32_t intValue);
//        void emit(Opcode op, uint32_t lineNumber, float floatValue);
//        void emit(Opcode op, uint32_t lineNumber, bool boolValue);
//        void emit(Opcode op, uint32_t lineNumber, const std::string &stringValue);
//        void emit(Opcode op, uint32_t lineNumber);

//        char advance(Scanner &scanner);
//        bool match(Scanner &scanner, TokenType expected);
//        void error(const std::string &message, int line = 0, int start = 0);
//};

// builtin_functions.hh
#pragma once
#include "function.hh"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>

class BuiltinFunctions
{
public:
    static void registerBuiltins(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        registerLen(functions, typeSystem);
        registerTime(functions, typeSystem);
        registerDebug(functions, typeSystem);
        // Add new function registrations
        registerInput(functions, typeSystem);
        registerMathFunctions(functions, typeSystem);
        registerTypeFunction(functions, typeSystem);
        registerAssert(functions, typeSystem);
        registerRound(functions, typeSystem);
        registerSleep(functions, typeSystem);
    }

private:
    static void registerLen(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // len(value: string|list|dict) -> int
        std::vector<ParameterInfo> lenParams = {
            ParameterInfo("value", makeAnyType(), false) // Required parameter
        };

        auto lenImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            const auto &value = args[0];
            size_t length = 0;

            switch (value->type->tag) {
            case TypeTag::String:
                length = std::get<std::string>(value->data).length();
                break;
            default:
                throw std::runtime_error(
                    "Type error: len() requires string, list, or dict argument");
            }

            return makeIntValue(static_cast<int32_t>(length));
        };

        functions.addBuiltinFunction("len", lenParams, makeType(TypeTag::Int), lenImpl);
    }

    static void registerTime(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // time() -> float
        // Returns current Unix timestamp in seconds with microsecond precision
        std::vector<ParameterInfo> timeParams = {}; // No parameters

        auto timeImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration);
            double seconds = micros.count() / 1000000.0;

            auto value = std::make_shared<Value>();
            value->type = std::make_shared<Type>(TypeTag::Float64);
            value->data = seconds;
            return value;
        };

        functions.addBuiltinFunction("time", timeParams, makeType(TypeTag::Float64), timeImpl);
    }

    static void registerDebug(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // debug(value: any, showType: bool = true) -> string
        std::vector<ParameterInfo> debugParams = {
            ParameterInfo("value", makeAnyType(), false), // Required value to debug
            ParameterInfo("showType",
                          makeType(TypeTag::Bool),
                          true,
                          makeBoolValue(true)) // Optional flag to show type info
        };

        auto debugImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            const auto &value = args[0];
            bool showType = std::get<bool>(args[1]->data);

            std::stringstream output;

            // Helper function to format type information
            auto formatType = [](const TypePtr &type) -> std::string {
                std::stringstream ss;
                ss << "<" << typeTagToString(type->tag);
                // if (type->elementType) {
                //     ss << " of " << typeTagToString(type->elementType->tag);
                // }
                ss << ">";
                return ss.str();
            };

            // Add type information if requested
            if (showType) {
                output << formatType(value->type) << ": ";
            }

            // Format the value based on its type
            switch (value->type->tag) {
            case TypeTag::Nil:
                output << "nil";
                break;
            case TypeTag::Int:
            case TypeTag::Int32:
                output << std::get<int32_t>(value->data);
                break;
            case TypeTag::Float32:
            case TypeTag::Float64: {
                auto val = std::get<double>(value->data);
                output << std::fixed << std::setprecision(6) << val;
                break;
            }
            case TypeTag::Bool:
                output << (std::get<bool>(value->data) ? "true" : "false");
                break;
            case TypeTag::String: {
                auto str = std::get<std::string>(value->data);
                output << "\"" << str << "\"";
                break;
            }
            default:
                output << "<unknown>";
            }

            return makeStringValue(output.str());
        };

        functions.addBuiltinFunction("debug", debugParams, makeType(TypeTag::String), debugImpl);
    }

    static void registerInput(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // input(prompt: string = "") -> string
        std::vector<ParameterInfo> inputParams = {
            ParameterInfo("prompt", makeType(TypeTag::String), true, makeStringValue(""))};

        auto inputImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            // Print prompt if provided
            std::cout << std::get<std::string>(args[0]->data);
            std::cout.flush();

            std::string input;
            std::getline(std::cin, input);
            return makeStringValue(input);
        };

        functions.addBuiltinFunction("input", inputParams, makeType(TypeTag::String), inputImpl);
    }

    static void registerMathFunctions(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // abs(x: int|float) -> int|float
        std::vector<ParameterInfo> absParams = {
            ParameterInfo("x", makeType(TypeTag::Float64), false)};

        auto absImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            const auto &value = args[0];
            if (value->type->tag == TypeTag::Int || value->type->tag == TypeTag::Int32) {
                int32_t val = std::get<int32_t>(value->data);
                return makeIntValue(std::abs(val));
            } else {
                double val = std::get<double>(value->data);
                return makeFloatValue(std::abs(val));
            }
        };

        functions.addBuiltinFunction("abs", absParams, makeType(TypeTag::Float64), absImpl);

        // sqrt(x: float) -> float
        std::vector<ParameterInfo> sqrtParams = {
            ParameterInfo("x", makeType(TypeTag::Float64), false)};

        auto sqrtImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            double val = std::get<double>(args[0]->data);
            if (val < 0) {
                throw std::runtime_error("Math error: sqrt() domain error");
            }
            return makeFloatValue(std::sqrt(val));
        };

        functions.addBuiltinFunction("sqrt", sqrtParams, makeType(TypeTag::Float64), sqrtImpl);
    }

    static void registerTypeFunction(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // type(value: any) -> string
        std::vector<ParameterInfo> typeParams = {ParameterInfo("value", makeAnyType(), false)};

        auto typeImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            return makeStringValue(typeTagToString(args[0]->type->tag));
        };

        functions.addBuiltinFunction("type", typeParams, makeType(TypeTag::String), typeImpl);
    }

    static void registerAssert(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // assert(condition: bool, message: string = "") -> nil
        std::vector<ParameterInfo> assertParams
            = {ParameterInfo("condition", makeType(TypeTag::Bool), false),
               ParameterInfo("message", makeType(TypeTag::String), true, makeStringValue(""))};

        auto assertImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            if (!std::get<bool>(args[0]->data)) {
                std::string msg = std::get<std::string>(args[1]->data);
                throw std::runtime_error("Assertion failed" + (msg.empty() ? "" : ": " + msg));
            }
            return makeNilValue();
        };

        functions.addBuiltinFunction("assert", assertParams, makeType(TypeTag::Nil), assertImpl);
    }

    static void registerRound(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // round(x: float, places: int = 0) -> float
        std::vector<ParameterInfo> roundParams
            = {ParameterInfo("x", makeType(TypeTag::Float64), false),
               ParameterInfo("places", makeType(TypeTag::Int), true, makeIntValue(0))};

        auto roundImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            double value = std::get<double>(args[0]->data);
            int32_t places = std::get<int32_t>(args[1]->data);

            double multiplier = std::pow(10.0, places);
            double rounded = std::round(value * multiplier) / multiplier;

            return makeFloatValue(rounded);
        };

        functions.addBuiltinFunction("round", roundParams, makeType(TypeTag::Float64), roundImpl);
    }

    static void registerSleep(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // sleep(seconds: float) -> nil
        std::vector<ParameterInfo> sleepParams = {
            ParameterInfo("seconds", makeType(TypeTag::Float64), false)};

        auto sleepImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            double seconds = std::get<double>(args[0]->data);
            if (seconds < 0) {
                throw std::runtime_error("Sleep time cannot be negative");
            }

            // Convert to microseconds for more precise sleep
            auto microseconds = static_cast<int64_t>(seconds * 1000000);
            std::this_thread::sleep_for(std::chrono::microseconds(microseconds));

            return makeNilValue();
        };

        functions.addBuiltinFunction("sleep", sleepParams, makeType(TypeTag::Nil), sleepImpl);
    }

    // Helper function for value comparison
    static int compareValues(const ValuePtr &a, const ValuePtr &b)
    {
        if (a->type->tag != b->type->tag) {
            throw std::runtime_error("Cannot compare values of different types");
        }

        switch (a->type->tag) {
        case TypeTag::Int:
        case TypeTag::Int32: {
            int32_t va = std::get<int32_t>(a->data);
            int32_t vb = std::get<int32_t>(b->data);
            return (va > vb) - (va < vb);
        }
        case TypeTag::Float32:
        case TypeTag::Float64: {
            double va = std::get<double>(a->data);
            double vb = std::get<double>(b->data);
            return (va > vb) - (va < vb);
        }
        case TypeTag::String: {
            const auto &va = std::get<std::string>(a->data);
            const auto &vb = std::get<std::string>(b->data);
            return (va > vb) - (va < vb);
        }
        default:
            throw std::runtime_error("Unsupported type for comparison");
        }
    }

    static ValuePtr makeFloatValue(double x)
    {
        auto value = std::make_shared<Value>();
        value->type = std::make_shared<Type>(TypeTag::Float64);
        value->data = x;
        return value;
    }

    static TypePtr makeListType(TypePtr elementType)
    {
        auto type = std::make_shared<Type>(TypeTag::List);
        type->extra = ListType{elementType}; // Use the ListType struct from the extra variant
        return type;
    }

    // Helper functions to create types and values
    static TypePtr makeAnyType() { return std::make_shared<Type>(TypeTag::Any); }

    static ValuePtr makeNilValue()
    {
        auto value = std::make_shared<Value>();
        value->type = std::make_shared<Type>(TypeTag::Nil);
        return value;
    }

    static ValuePtr makeIntValue(int32_t n)
    {
        auto value = std::make_shared<Value>();
        value->type = std::make_shared<Type>(TypeTag::Int);
        value->data = n;
        return value;
    }

    static ValuePtr makeStringValue(const std::string &s)
    {
        auto value = std::make_shared<Value>();
        value->type = std::make_shared<Type>(TypeTag::String);
        value->data = s;
        return value;
    }

    static ValuePtr makeBoolValue(bool b)
    {
        auto value = std::make_shared<Value>();
        value->type = std::make_shared<Type>(TypeTag::Bool);
        value->data = b;
        return value;
    }

    static TypePtr makeType(TypeTag tag) { return std::make_shared<Type>(tag); }

    static std::string typeTagToString(TypeTag tag)
    {
        switch (tag) {
        case TypeTag::Nil:
            return "nil";
        case TypeTag::Bool:
            return "bool";
        case TypeTag::Int:
            return "int";
        case TypeTag::Int32:
            return "int32";
        case TypeTag::Float32:
            return "float32";
        case TypeTag::Float64:
            return "float64";
        case TypeTag::String:
            return "string";
        case TypeTag::List:
            return "list";
        case TypeTag::Dict:
            return "dict";
        case TypeTag::Function:
            return "function";
        case TypeTag::Any:
            return "any";
        default:
            return "unknown";
        }
    }
};
