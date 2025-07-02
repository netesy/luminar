// builtin_functions.hh
#pragma once
class Functions;
#include "types.hh"
#include "function_types.hh"
#include <variant>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>

template<typename FunctionRegistry>
class BuiltinFunctions
{
public:
    static void registerWith(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        registerLen(functions, typeSystem);
        registerTime(functions, typeSystem);
        registerNow(functions, typeSystem);
        registerDate(functions, typeSystem);
        registerDebug(functions, typeSystem);
        registerInput(functions, typeSystem);
        registerMathFunctions(functions, typeSystem);
        registerTypeFunction(functions, typeSystem);
        registerAssert(functions, typeSystem);
        registerRound(functions, typeSystem);
        registerSleep(functions, typeSystem);
        registerContract(functions, typeSystem);

       // registerPrint(functions, typeSystem);
        // registerListFunctions(functions, typeSystem);
        // registerFileOperations(functions, typeSystem);
    }

private:
    static void validateParameters(const std::vector<ValuePtr> &args,
                                   size_t expectedCount,
                                   const std::string &functionName) {
        if (args.size() < expectedCount) {
            throw std::runtime_error(functionName + "() missing required arguments");
        }

        // Check for null parameters
        for (size_t i = 0; i < args.size(); ++i) {
            if (!args[i]) {
                throw std::runtime_error(functionName + "() argument " + std::to_string(i) + " is null");
            }
        }
    }

    static void registerLen(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // len(value: string|list|dict) -> int
        std::vector<ParameterInfo> lenParams = {
            ParameterInfo("value", makeAnyType(), false) // Required parameter
        };

        auto lenImpl = [&functions](const std::vector<ValuePtr> &args) -> ValuePtr {
            const auto &value = args[0];
            size_t length = 0;

            switch (value->type->tag) {
            case TypeTag::String:
                length = std::get<std::string>(value->data).length();
                break;
            // case TypeTag::Int:
            //     length = std::get<int64_t>(value->data).
            case TypeTag::List:
                length = std::get<ListValue>(value->data).len();
                break;
            case TypeTag::Dict:
                length = std::get<DictValue>(value->data).len();
                break;
            // default:
                throw std::runtime_error(
                    "Type error: len() requires string, list, or dict argument");
            case TypeTag::Nil:
            case TypeTag::Bool:
            case TypeTag::Int:
            case TypeTag::Int8:
            case TypeTag::Int16:
            case TypeTag::Int32:
            case TypeTag::Int64:
            case TypeTag::UInt:
            case TypeTag::UInt8:
            case TypeTag::UInt16:
            case TypeTag::UInt32:
            case TypeTag::UInt64:
            case TypeTag::Float32:
            case TypeTag::Float64:
            case TypeTag::Enum:
            case TypeTag::Function:
            case TypeTag::Any:
            case TypeTag::Sum:
            case TypeTag::Union:
            case TypeTag::UserDefined:
                break;
            }

            return makeIntValue(static_cast<int32_t>(length));
        };

        functions.addBuiltinFunction("len", lenParams, makeType(TypeTag::Int), lenImpl);
    }

    static void registerTime(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // time() -> string
        // Returns the current date and time as a human-readable string
        std::vector<ParameterInfo> timeParams = {}; // No parameters

        auto timeImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            // Parameter count validation
            if (!args.empty()) {
                throw std::runtime_error("time() takes no arguments");
            }

            try {
                auto now = std::chrono::system_clock::now();
                auto now_time_t = std::chrono::system_clock::to_time_t(now);

                // Thread-safe time conversion with proper cross-platform handling
                std::tm tm_buf;
                std::tm* tm_ptr = nullptr;

#if defined(_WIN32) || defined(_MSC_VER)
                // Windows/MSVC: Use localtime_s
                errno_t err = localtime_s(&tm_buf, &now_time_t);
                if (err != 0) {
                    throw std::runtime_error("Failed to convert time: localtime_s error " + std::to_string(err));
                }
                tm_ptr = &tm_buf;
#elif defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
                // C11 optional extensions: Use localtime_s if available
                if (localtime_s(&now_time_t, &tm_buf) == nullptr) {
                    throw std::runtime_error("Failed to convert time: localtime_s returned null");
                }
                tm_ptr = &tm_buf;
#else \
    // POSIX systems: Use localtime_r
                tm_ptr = localtime_r(&now_time_t, &tm_buf);
                if (!tm_ptr) {
                    throw std::runtime_error("Failed to convert time: localtime_r returned null");
                }
#endif

                // Format time with adequate buffer size
                constexpr size_t BUFFER_SIZE = 64; // Generous buffer size
                char buffer[BUFFER_SIZE];

                size_t result = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_ptr);
                if (result == 0) {
                    throw std::runtime_error("Failed to format time: strftime returned 0");
                }

                std::string timeStr(buffer);

// Optional: Debug output (remove in production)
#ifdef DEBUG_TIME_FUNCTION
                std::cerr << "DEBUG: time() returning: '" << timeStr << "'" << std::endl;
#endif

                return makeStringValue(timeStr);

            } catch (const std::exception& e) {
#ifdef DEBUG_TIME_FUNCTION
                std::cerr << "DEBUG: time() exception: " << e.what() << std::endl;
#endif
                throw; // Re-throw the exception
            }
        };

        functions.addBuiltinFunction("time", timeParams, makeType(TypeTag::String), timeImpl);
    }

    static void registerNow(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // now() -> float
        // Returns current Unix timestamp in seconds with microsecond precision
        std::vector<ParameterInfo> timeParams = {}; // No parameters

        auto timeImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            // Get the current time
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration);

            // Convert to seconds with fixed-point precision
            double seconds = static_cast<double>(micros.count()) / 1'000'000.0;

            // Round and format the timestamp to avoid scientific notation
            double roundedSeconds = std::round(seconds * 1'000'000) / 1'000'000;

            // Create the return value
            return makeFloatValue(roundedSeconds);
        };

        functions.addBuiltinFunction("now", timeParams, makeType(TypeTag::Float64), timeImpl);
    }

    static void registerDate(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // date(format: string, timestamp: int = current time) -> string
        std::vector<ParameterInfo> dateParams
            = {ParameterInfo("format", makeType(TypeTag::String), false),
               ParameterInfo("timestamp", makeType(TypeTag::Int), true, makeIntValue(1))};

        auto dateImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "date");
            const std::string &format = std::get<std::string>(args[0]->data);
            int64_t timestamp = 1;

            if (args.size() > 1) {
                timestamp = std::get<int64_t>(args[1]->data);
            } else {
                timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            }

            // Convert timestamp to time_t and then to tm
            std::time_t time = static_cast<std::time_t>(timestamp);
            std::tm tm;
#ifdef _WIN32
            localtime_s(&tm, &time); // Windows-specific
#else
            localtime_r(&time, &tm); // POSIX-compliant
#endif

            // Preallocate buffer to hold the formatted date
            char buffer[100];
            if (std::strftime(buffer, sizeof(buffer), format.c_str(), &tm) == 0) {
                throw std::runtime_error("Failed to format date");
            }

            return makeStringValue(std::string(buffer));
        };

        functions.addBuiltinFunction("date", dateParams, makeType(TypeTag::String), dateImpl);
    }

    static void registerDebug(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // debug(value: any, showType: bool = true) -> string
        std::vector<ParameterInfo> debugParams
            = {ParameterInfo("value", makeAnyType(), false),
               ParameterInfo("showType", makeType(TypeTag::Bool), true, makeBoolValue(true))};

        auto debugImpl = [&functions](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "debug");
            const auto &value = args[0];
            bool showType = std::get<bool>(args[1]->data);

            std::stringstream output;
            auto formatType = [](const TypePtr &type) -> std::string {
                std::stringstream ss;
                ss << "<" << typeTagToString(type->tag) << ">";
                return ss.str();
            };

            if (showType) {
                output << formatType(value->type) << ": ";
            }

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
                // case TypeTag::List: {
                // const auto &list = std::get<ListValue>(value->data);
                // output << "[";
                // for (size_t i = 0; i < list.size(); ++i) {
                //     if (i > 0)
                //         output << ", ";
                //     // Recursive debug call for each element
                //     std::vector<ValuePtr> recursiveArgs = {list[i], functions.getParameter("value")};
                //     auto elemDebug = debugImpl(recursiveArgs);
                //     output << std::get<std::string>(elemDebug->data);
                // }
                // output << "]";
                // break;
                // }
                // case TypeTag::Dict: {
                //     const auto &dict = std::get<DictValue>(value->data);
                //     output << "{";
                //     bool first = true;
                //     for (const auto &[key, val] : dict) {
                //         if (!first)
                //             output << ", ";
                //         output << "\"" << key << "\": ";
                //         // Recursive debug call for each value
                //         std::vector<ValuePtr> recursiveArgs = {val, functions.getParameter("value")};
                //         auto elemDebug = debugImpl(recursiveArgs);
                //         output << std::get<std::string>(elemDebug->data);
                //         first = false;
                //     }
                //     output << "}";
                //     break;
                // }
                // case TypeTag::Function: {
                //     const auto &func = std::get<FunctionValue>(value->data);
                //     output << "<function " << func.name << ">";
                //     break;
                // }
            default:
                output << "<unknown>";
            }

            return makeStringValue(output.str());
        };

        functions.addBuiltinFunction("debug", debugParams, makeType(TypeTag::String), debugImpl);
    }

    static void registerInput(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // input(prompt: string = "") -> string
        std::vector<ParameterInfo> inputParams = {
            ParameterInfo("prompt", makeType(TypeTag::String), true, makeStringValue(""))};

        auto inputImpl = [&functions](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "input");
            // Print prompt if provided
            std::cout << std::get<std::string>(args[0]->data);
            std::cout.flush();

            std::string input;
            std::getline(std::cin, input);
            return makeStringValue(input);
        };

        functions.addBuiltinFunction("input", inputParams, makeType(TypeTag::String), inputImpl);
    }

    static void registerMathFunctions(FunctionRegistry &functions,
                                      std::shared_ptr<TypeSystem> typeSystem)
    {
        // abs(x: int|float) -> int|float
        std::vector<ParameterInfo> absParams = {
            ParameterInfo("value", makeType(TypeTag::Float64), false)};

        auto absImpl = [&functions](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "abs");
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
            ParameterInfo("value", makeType(TypeTag::Float64), false)};

        auto sqrtImpl = [&functions](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "sqrt");
            double val = std::get<double>(args[0]->data);
            if (val < 0) {
                throw std::runtime_error("Math error: sqrt() domain error");
            }
            return makeFloatValue(std::sqrt(val));
        };

        functions.addBuiltinFunction("sqrt", sqrtParams, makeType(TypeTag::Float64), sqrtImpl);
    }

    static void registerTypeFunction(FunctionRegistry &functions,
                                     std::shared_ptr<TypeSystem> typeSystem)
    {
        // type(value: any) -> string
        std::vector<ParameterInfo> typeParams = {ParameterInfo("value", makeAnyType(), false)};

        auto typeImpl = [&functions](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "type");
            return makeStringValue(typeTagToString(args[0]->type->tag));
        };

        functions.addBuiltinFunction("type", typeParams, makeType(TypeTag::String), typeImpl);
    }

    static void registerAssert(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // assert(condition: bool, message: string = "") -> nil
        std::vector<ParameterInfo> assertParams
            = {ParameterInfo("condition", makeType(TypeTag::Bool), false),
               ParameterInfo("message", makeType(TypeTag::String), true, makeStringValue(""))};

        auto assertImpl = [&functions](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "assert");
            if (!std::get<bool>(args[0]->data)) {
                std::string msg = std::get<std::string>(args[1]->data);
                throw std::runtime_error("Assertion failed" + (msg.empty() ? "" : ": " + msg));
            }
            return makeNilValue();
        };

        functions.addBuiltinFunction("assert", assertParams, makeType(TypeTag::Nil), assertImpl);
    }

    static void registerRound(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // round(value: float, places: int = 0) -> float
        std::vector<ParameterInfo> roundParams
            = {ParameterInfo("value", makeType(TypeTag::Float64), false),
               ParameterInfo("places", makeType(TypeTag::Int), true, makeIntValue(0))};

        auto roundImpl = [&functions](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "round");
            double value = std::get<double>(args[0]->data);
            int64_t places = std::get<int64_t>(args[1]->data);

            double multiplier = std::pow(10.0, places);
            double rounded = std::round(value * multiplier) / multiplier;

            return makeFloatValue(rounded);
        };

        functions.addBuiltinFunction("round", roundParams, makeType(TypeTag::Float64), roundImpl);
    }

    static void registerSleep(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // sleep(seconds: float) -> nil
        std::vector<ParameterInfo> sleepParams = {
            ParameterInfo("seconds", makeType(TypeTag::Float64), false)};

        auto sleepImpl = [&functions](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "sleep");
            // Direct access of the first argument instead of using getParameter
            if (args.empty()) {
                throw std::runtime_error("Sleep function requires one argument");
            }

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


    static void registerContract(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // contract(condition: bool, message: string, action: string = "throw") -> nil
        std::vector<ParameterInfo> contractParams = {
            ParameterInfo("condition", makeType(TypeTag::Bool), false),
            ParameterInfo("message", makeType(TypeTag::String), false),
            ParameterInfo("action", makeType(TypeTag::String), true, makeStringValue("throw"))
        };

        auto contractImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 2, "contract");
            bool condition = std::get<bool>(args[0]->data);
            std::string message = std::get<std::string>(args[1]->data);
            std::string action = std::get<std::string>(args[2]->data);

            if (!condition) {
                if (action == "throw") {
                    throw std::runtime_error("Contract Violation: " + message);
                } else if (action == "warn") {
                    std::cerr << "Warning: " << message << std::endl;
                } else if (action == "log") {
                    std::ofstream logFile("contract.log", std::ios::app);
                    logFile << "Contract Violation: " << message << std::endl;
                } else {
                    throw std::runtime_error("Invalid contract action: " + action);
                }
            }

            return makeNilValue();
        };

        functions.addBuiltinFunction("contract", contractParams, makeType(TypeTag::Nil), contractImpl);
    }



    static void registerPrint(FunctionRegistry &functions, std::shared_ptr<TypeSystem> typeSystem)
    {
        // print(value: any, end: string = "\n") -> nil
        std::vector<ParameterInfo> printParams = {
            ParameterInfo("value", makeAnyType(), false),
            ParameterInfo("end", makeType(TypeTag::String), true, makeStringValue("\n"))
        };

        auto printImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
            validateParameters(args, 1, "print");

            // Define the printValue function before using it
            std::function<void(const ValuePtr&, bool)> printValue;
            printValue = [&printValue](const ValuePtr &val, bool isTopLevel = true) {
                switch (val->type->tag) {
                case TypeTag::Nil:
                    std::cout << "nil";
                    break;
                case TypeTag::Bool:
                    std::cout << (std::get<bool>(val->data) ? "true" : "false");
                    break;
                case TypeTag::Int:
                case TypeTag::Int32:
                    std::cout << std::get<int32_t>(val->data);
                    break;
                case TypeTag::Float32:
                case TypeTag::Float64:
                    std::cout << std::get<double>(val->data);
                    break;
                case TypeTag::String:
                    if (isTopLevel) {
                        std::cout << std::get<std::string>(val->data);
                    } else {
                        std::cout << "\"" << std::get<std::string>(val->data) << "\"";
                    }
                    break;
                case TypeTag::List: {
                    const auto &list = std::get<ListValue>(val->data);
                    std::cout << "[";
                    for (size_t i = 0; i < list.elements.size(); ++i) {
                        printValue(list.elements[i], false);
                        if (i < list.elements.size() - 1) {
                            std::cout << ", ";
                        }
                    }
                    std::cout << "]";
                    break;
                }
                case TypeTag::Dict: {
                    const auto &dict = std::get<DictValue>(val->data);
                    std::cout << "{";
                    bool first = true;
                    for (const auto &[key, value] : dict.elements) {
                        if (!first) std::cout << ", ";
                        std::cout << "\"" << key << "\": ";
                        printValue(value, false);
                        first = false;
                    }
                    std::cout << "}";
                    break;
                }
                default:
                    std::cout << "<" << typeTagToString(val->type->tag) << ">";
                }
            };

            // Print the value
            printValue(args[0], true);

            // Print end string (defaults to "\n")
            std::string end = "\n";
            if (args.size() > 1) {
                end = std::get<std::string>(args[1]->data);
            }
            std::cout << end;
            std::cout.flush();

            return makeNilValue();
        };

        functions.addBuiltinFunction("print", printParams, makeType(TypeTag::Nil), printImpl);
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
        case TypeTag::Int8:
            return "int8";
        case TypeTag::Int16:
            return "int16";
        case TypeTag::Int32:
            return "int32";
        case TypeTag::Int64:
            return "int64";
        case TypeTag::UInt:
            return "uint";
        case TypeTag::UInt8:
            return "uint8";
        case TypeTag::UInt16:
            return "uint16";
        case TypeTag::UInt32:
            return "uint32";
        case TypeTag::UInt64:
            return "uint64";
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
        case TypeTag::Sum:
            return "sum";
        case TypeTag::Union:
            return "union";
        case TypeTag::UserDefined:
            return "userdefined";
        case TypeTag::Enum:
            return "enum";
        default:
            return "unknown";
        }
    }
};
