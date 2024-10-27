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
        // registerPrint(functions, typeSystem);
        registerLen(functions, typeSystem);
        registerTime(functions, typeSystem);
        registerDebug(functions, typeSystem);
        // Add new function registrations
        registerInput(functions, typeSystem);
        registerMathFunctions(functions, typeSystem);
        //   registerListFunctions(functions, typeSystem);
        registerTypeFunction(functions, typeSystem);
        registerAssert(functions, typeSystem);
        registerRound(functions, typeSystem);
        registerSleep(functions, typeSystem);
        // registerFileOperations(functions, typeSystem);
    }

private:
    // static void registerPrint(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    // {
    //     // print([value: any], end: string = "\n")
    //     std::vector<ParameterInfo> printParams = {
    //         ParameterInfo("value", makeAnyType(), true, makeNilValue()), // Optional value
    //         ParameterInfo("end",
    //                       makeType(TypeTag::String),
    //                       true,
    //                       makeStringValue("\n")) // Optional newline
    //     };

    //     auto printImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
    //         std::stringstream output;

    //         // Handle the value to print
    //         if (args[0]->type->tag != TypeTag::Nil) {
    //             switch (args[0]->type->tag) {
    //             case TypeTag::Int:
    //             case TypeTag::Int32:
    //                 output << std::get<int32_t>(args[0]->data);
    //                 break;
    //             case TypeTag::Float32:
    //             case TypeTag::Float64:
    //                 output << std::get<double>(args[0]->data);
    //                 break;
    //             case TypeTag::Bool:
    //                 output << (std::get<bool>(args[0]->data) ? "true" : "false");
    //                 break;
    //             case TypeTag::String:
    //                 output << std::get<std::string>(args[0]->data);
    //                 break;
    //             case TypeTag::List: {
    //                 const auto &list = std::get<ListValue>(args[0]->data);
    //                 output << "[";
    //                 for (size_t i = 0; i < list.size(); ++i) {
    //                     if (i > 0)
    //                         output << ", ";
    //                     // Recursive call to print for each element
    //                     std::vector<ValuePtr> recursiveArgs = {list[i]};
    //                     printImpl(recursiveArgs);
    //                 }
    //                 output << "]";
    //                 break;
    //             }
    //             case TypeTag::Dict: {
    //                 const auto &dict = std::get<DictValue>(args[0]->data);
    //                 output << "{";
    //                 bool first = true;
    //                 for (const auto &[key, value] : dict) {
    //                     if (!first)
    //                         output << ", ";
    //                     output << key << ": ";
    //                     // Recursive call to print for each value
    //                     std::vector<ValuePtr> recursiveArgs = {value};
    //                     printImpl(recursiveArgs);
    //                     first = false;
    //                 }
    //                 output << "}";
    //                 break;
    //             }
    //             default:
    //                 output << "<unprintable>";
    //             }
    //         }

    //         // Add the end string (usually newline)
    //         output << std::get<std::string>(args[1]->data);

    //         // Actually print to stdout
    //         std::cout << output.str();
    //         std::cout.flush();

    //         return makeNilValue();
    //     };

    //     functions.addBuiltinFunction("print", printParams, makeType(TypeTag::Nil), printImpl);
    // }

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
            // case TypeTag::List:
            //     length = std::get<ListValue>(value->data).size();
            //     break;
            // case TypeTag::Dict:
            //     length = std::get<DictValue>(value->data).size();
            //     break;
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
                // case TypeTag::List: {
                // const auto &list = std::get<ListValue>(value->data);
                // output << "[";
                // for (size_t i = 0; i < list.size(); ++i) {
                //     if (i > 0)
                //         output << ", ";
                //     // Recursive debug call for each element
                //     std::vector<ValuePtr> recursiveArgs = {list[i], args[1]};
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
                //         std::vector<ValuePtr> recursiveArgs = {val, args[1]};
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

    // static void registerListFunctions(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    // {
    //     // min(list: list) -> any
    //     std::vector<ParameterInfo> minParams = {
    //         ParameterInfo("list", makeListType(makeAnyType()), false)};

    //     auto minImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
    //         const auto &list = std::get<ListValue>(args[0]->data);
    //         if (list.empty()) {
    //             throw std::runtime_error("min() arg is an empty sequence");
    //         }

    //         ValuePtr minVal = list[0];
    //         for (size_t i = 1; i < list.size(); i++) {
    //             if (compareValues(list[i], minVal) < 0) {
    //                 minVal = list[i];
    //             }
    //         }
    //         return minVal;
    //     };

    //     functions.addBuiltinFunction("min", minParams, makeAnyType(), minImpl);

    //     // max(list: list) -> any
    //     std::vector<ParameterInfo> maxParams = {
    //         ParameterInfo("list", makeListType(makeAnyType()), false)};

    //     auto maxImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
    //         const auto &list = std::get<ListValue>(args[0]->data);
    //         if (list.empty()) {
    //             throw std::runtime_error("max() arg is an empty sequence");
    //         }

    //         ValuePtr maxVal = list[0];
    //         for (size_t i = 1; i < list.size(); i++) {
    //             if (compareValues(list[i], maxVal) > 0) {
    //                 maxVal = list[i];
    //             }
    //         }
    //         return maxVal;
    //     };

    //     functions.addBuiltinFunction("max", maxParams, makeAnyType(), maxImpl);

    //     // sum(list: list) -> number
    //     std::vector<ParameterInfo> sumParams = {
    //         ParameterInfo("list", makeListType(makeType(TypeTag::Float64)), false)};

    //     auto sumImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
    //         const auto &list = std::get<ListValue>(args[0]->data);
    //         double sum = 0.0;
    //         for (const auto &val : list) {
    //             if (val->type->tag == TypeTag::Int || val->type->tag == TypeTag::Int32) {
    //                 sum += std::get<int32_t>(val->data);
    //             } else {
    //                 sum += std::get<double>(val->data);
    //             }
    //         }
    //         return makeFloatValue(sum);
    //     };

    //     functions.addBuiltinFunction("sum", sumParams, makeType(TypeTag::Float64), sumImpl);
    // }

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

    // static void registerFileOperations(Functions &functions, std::shared_ptr<TypeSystem> typeSystem)
    // {
    //     // File handle type for managing open files
    //     struct FileHandle
    //     {
    //         std::fstream file;
    //         std::string mode;
    //         std::string path;
    //     };

    //     // Custom type for file handles
    //     auto fileType = std::make_shared<Type>(TypeTag::Any); // Using Any as a placeholder

    //     // open(path: string, mode: string = "r") -> file
    //     std::vector<ParameterInfo> openParams
    //         = {ParameterInfo("path", makeType(TypeTag::String), false),
    //            ParameterInfo("mode", makeType(TypeTag::String), true, makeStringValue("r"))};

    //     auto openImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
    //         std::string path = std::get<std::string>(args[0]->data);
    //         std::string mode = std::get<std::string>(args[1]->data);

    //         // Convert mode string to fstream flags
    //         std::ios_base::openmode flags = std::ios_base::binary; // Always use binary mode

    //         if (mode == "r") {
    //             flags |= std::ios_base::in;
    //         } else if (mode == "w") {
    //             flags |= std::ios_base::out | std::ios_base::trunc;
    //         } else if (mode == "a") {
    //             flags |= std::ios_base::out | std::ios_base::app;
    //         } else if (mode == "r+") {
    //             flags |= std::ios_base::in | std::ios_base::out;
    //         } else if (mode == "w+") {
    //             flags |= std::ios_base::in | std::ios_base::out | std::ios_base::trunc;
    //         } else if (mode == "a+") {
    //             flags |= std::ios_base::in | std::ios_base::out | std::ios_base::app;
    //         } else {
    //             throw std::runtime_error("Invalid file mode: " + mode);
    //         }

    //         auto handle = std::make_shared<FileHandle>();
    //         handle->path = path;
    //         handle->mode = mode;
    //         handle->file.open(path, flags);

    //         if (!handle->file.is_open()) {
    //             throw std::runtime_error("Failed to open file: " + path);
    //         }

    //         auto value = std::make_shared<Value>();
    //         value->type = fileType;
    //         value->data = handle;
    //         return value;
    //     };

    //     functions.addBuiltinFunction("open", openParams, fileType, openImpl);

    //     // read(file: file, size: int = -1) -> string
    //     std::vector<ParameterInfo> readParams
    //         = {ParameterInfo("file", fileType, false),
    //            ParameterInfo("size", makeType(TypeTag::Int), true, makeIntValue(-1))};

    //     auto readImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
    //         auto handle = std::get<std::shared_ptr<FileHandle>>(args[0]->data);
    //         int32_t size = std::get<int32_t>(args[1]->data);

    //         if (!handle->file.is_open()) {
    //             throw std::runtime_error("File is not open");
    //         }

    //         if (handle->mode.find('r') == std::string::npos
    //             && handle->mode.find('+') == std::string::npos) {
    //             throw std::runtime_error("File not opened for reading");
    //         }

    //         std::string content;
    //         if (size < 0) {
    //             // Read entire file
    //             std::stringstream buffer;
    //             buffer << handle->file.rdbuf();
    //             content = buffer.str();
    //         } else {
    //             // Read specified number of bytes
    //             content.resize(size);
    //             handle->file.read(&content[0], size);
    //             content.resize(handle->file.gcount()); // Resize to actual bytes read
    //         }

    //         return makeStringValue(content);
    //     };

    //     functions.addBuiltinFunction("read", readParams, makeType(TypeTag::String), readImpl);

    //     // write(file: file, data: string) -> int
    //     std::vector<ParameterInfo> writeParams = {ParameterInfo("file", fileType, false),
    //                                               ParameterInfo("data",
    //                                                             makeType(TypeTag::String),
    //                                                             false)};

    //     auto writeImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
    //         auto handle = std::get<std::shared_ptr<FileHandle>>(args[0]->data);
    //         std::string data = std::get<std::string>(args[1]->data);

    //         if (!handle->file.is_open()) {
    //             throw std::runtime_error("File is not open");
    //         }

    //         if (handle->mode.find('w') == std::string::npos
    //             && handle->mode.find('a') == std::string::npos
    //             && handle->mode.find('+') == std::string::npos) {
    //             throw std::runtime_error("File not opened for writing");
    //         }

    //         handle->file.write(data.c_str(), data.length());
    //         handle->file.flush();

    //         return makeIntValue(static_cast<int32_t>(data.length()));
    //     };

    //     functions.addBuiltinFunction("write", writeParams, makeType(TypeTag::Int), writeImpl);

    //     // close(file: file) -> nil
    //     std::vector<ParameterInfo> closeParams = {ParameterInfo("file", fileType, false)};

    //     auto closeImpl = [](const std::vector<ValuePtr> &args) -> ValuePtr {
    //         auto handle = std::get<std::shared_ptr<FileHandle>>(args[0]->data);

    //         if (handle->file.is_open()) {
    //             handle->file.close();
    //         }

    //         return makeNilValue();
    //     };

    //     functions.addBuiltinFunction("close", closeParams, makeType(TypeTag::Nil), closeImpl);
    // }

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
