// debugger.h
#pragma once

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum class InterpretationStage { SCANNING, PARSING, SYNTAX, SEMANTIC, INTERPRETING, COMPILING };

class Debugger
{
    Debugger(const std::string &sourceCode)
        : sourceCode(splitLines(sourceCode))
    {}

public:
    static void debugInfo(const std::string &errorMessage,
                          int lineNumber,
                          int position,
                          InterpretationStage stage,
                          const std::string &expectedValue = "")
    {
        std::cerr << "Debug Info (" << stageToString(stage) << "):" << std::endl;
        std::cerr << "Line number: " << lineNumber << ", Position: " << position << std::endl;
        std::cerr << "Error: " << errorMessage << std::endl;
        if (!expectedValue.empty()) {
            std::cerr << "Expected value: " << expectedValue << std::endl;
        }
        std::cerr << "Time: " << getTime() << std::endl;
        // Show the line of code and highlight the problematic part
        if (lineNumber >= 1 && lineNumber <= sourceCode.size()) {
            std::cerr << "\nCode:\n" << sourceCode[lineNumber - 1] << std::endl;
            std::cerr << std::string(position - 1, ' ') << "^" << std::endl;
        }

        std::cerr << "Suggestion: " << getSuggestion(errorMessage, expectedValue) << std::endl;
        std::cerr << "Sample Solution: "
                  << getSampleSolution(errorMessage, expectedValue, lineNumber, position)
                  << "\n ----------------END----------------\n"
                  << std::endl;
    }

    static void error(const std::string &errorMessage,
                      int lineNumber,
                      int position,
                      InterpretationStage stage,
                      const std::string &token = "",
                      const std::string &expectedValue = "")
    {
        std::cerr << "\n ----------------DEBUG----------------\n"
                  << "Error at line " << lineNumber << ", position " << position << " ("
                  << stageToString(stage) << "): " << errorMessage << std::endl;
        if (!token.empty()) {
            std::cerr << "Token: " << token << std::endl;
        }
        if (!expectedValue.empty()) {
            std::cerr << "Expected value: " << expectedValue << std::endl;
        }
        std::cerr << "Time: " << getTime() << std::endl;
        std::cerr << "Suggestion: " << getSuggestion(errorMessage, expectedValue)
                  << "\n ----------------END----------------\n"
                  << std::endl;

        std::ofstream logfile("log.txt", std::ios_base::app); // Open log file for appending
        if (!logfile.is_open()) {
            std::cerr << "Failed to open log file." << std::endl;
            return;
        }

        logfile << "\n ----------------DEBUG----------------\n"
                << "Error at line " << lineNumber << ", position " << position << " ("
                << stageToString(stage) << "): " << errorMessage << std::endl;

        if (!token.empty()) {
            logfile << "Token: " << token << std::endl;
        }
        if (!expectedValue.empty()) {
            logfile << "Expected value: " << expectedValue << std::endl;
        }

        logfile << "Time: " << getTime() << std::endl;

        // Show the line of code and highlight the problematic part
        if (lineNumber >= 1 && lineNumber <= sourceCode.size()) {
            logfile << "\nCode:\n" << sourceCode[lineNumber - 1] << std::endl;
            logfile << std::string(position - 1, ' ') << "^" << std::endl;
        }

        logfile << "Suggestion: " << getSuggestion(errorMessage, expectedValue) << std::endl;
        logfile << "Sample Solution: "
                << getSampleSolution(errorMessage, expectedValue, lineNumber, position)
                << "\n ----------------END----------------\n"
                << std::endl;

        logfile.close(); // Close the log file
        // exit(4);
    }

private:
    std::vector<std::string> sourceCode;

    static std::vector<std::string> splitLines(const std::string &sourceCode)
    {
        std::vector<std::string> lines;
        std::stringstream ss(sourceCode);
        std::string line;
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }
        return lines;
    }
    static std::string getTime()
    {
        auto current_time = std::chrono::system_clock::now();
        std::time_t current_time_t = std::chrono::system_clock::to_time_t(current_time);
        return std::ctime(&current_time_t);
    }

    static std::string getSuggestion(const std::string &errorMessage,
                                     const std::string &expectedValue = "")
    {
        // Provide suggestions based on the error message and expected value
        // Provide suggestions based on the error message and expected value
        if (errorMessage.find("Invalid character") != std::string::npos) {
            return "Check for invalid characters in your code.";
        } else if (errorMessage.find("Variable/function not found") != std::string::npos) {
            return "Check the spelling of the variable or function name, or make sure it has been "
                   "declared or defined before use.";
        } else if (errorMessage.find("Invalid factor") != std::string::npos) {
            return "Check the expression to ensure it follows the correct syntax.";
        } else if (errorMessage.find("Unexpected token") != std::string::npos) {
            if (!expectedValue.empty()) {
                return "Expected '" + expectedValue
                       + "'. Ensure the syntax matches the expected pattern.";
            }
            return "Check your code for syntax errors.";
        } else if (errorMessage.find("Expected") != std::string::npos) {
            return "Ensure the correct syntax is followed. " + errorMessage;
        } else if (errorMessage.find("Invalid value stack for unary operation")
                   != std::string::npos) {
            return "Ensure the stack has sufficient values for the operation.";
        } else if (errorMessage.find("Invalid value stack for binary operation")
                   != std::string::npos) {
            return "Ensure the stack has two values for the binary operation.";
        } else if (errorMessage.find("Unsupported type for NEGATE operation") != std::string::npos) {
            return "NEGATE operation supports only int32_t and double types.";
        } else if (errorMessage.find("Unsupported type for NOT operation") != std::string::npos) {
            return "NOT operation supports only bool type.";
        } else if (errorMessage.find("Division by zero") != std::string::npos) {
            return "Ensure the divisor is not zero.";
        } else if (errorMessage.find("Modulo by zero") != std::string::npos) {
            return "Ensure the divisor is not zero.";
        } else if (errorMessage.find("Unsupported types for binary operation")
                   != std::string::npos) {
            return "Binary operations support int32_t and double types.";
        } else if (errorMessage.find("Insufficient value stack for logical operation")
                   != std::string::npos) {
            return "Ensure the stack has two values for the logical operation.";
        } else if (errorMessage.find("Unsupported types for logical operation")
                   != std::string::npos) {
            return "Logical operations support only bool type.";
        } else if (errorMessage.find("Insufficient value stack for comparison operation")
                   != std::string::npos) {
            return "Ensure the stack has two values for the comparison operation.";
        } else if (errorMessage.find("Unsupported types for comparison operation")
                   != std::string::npos) {
            return "Comparison operations support int32_t and double types.";
        } else if (errorMessage.find("Invalid variable index") != std::string::npos) {
            return "Ensure the variable index is within the valid range.";
        } else if (errorMessage.find("value stack underflow") != std::string::npos) {
            return "Ensure there are enough values on the stack for the operation.";
        } else if (errorMessage.find("Invalid jump offset type") != std::string::npos) {
            return "Ensure the jump offset is of type int32_t.";
        } else if (errorMessage.find("JUMP_IF_FALSE requires a boolean condition")
                   != std::string::npos) {
            return "Ensure the condition for JUMP_IF_FALSE is a boolean.";
        } else {
            return "Check your code for errors.";
        }
    }

    static std::string getSampleSolution(const std::string &errorMessage,
                                         const std::string &expectedValue,
                                         int lineNumber,
                                         int position)
    {
        // Provide a sample solution based on the error message and expected value
        if (errorMessage.find("Invalid character") != std::string::npos) {
            return "Check for invalid characters such as '$', '$', or '$' in your code.";
        } else if (errorMessage.find("Variable/function not found") != std::string::npos) {
            return "Check the spelling of the variable or function name, or make sure it has been "
                   "declared or defined before use.";
        } else if (errorMessage.find("Invalid factor") != std::string::npos) {
            return "Ensure the expression follows the correct syntax, with valid operators and "
                   "operands.";
        } else if (errorMessage.find("Unexpected token") != std::string::npos) {
            if (!expectedValue.empty()) {
                return "Expected '" + expectedValue
                       + "'. Ensure the syntax matches the expected pattern.";
            }
            return "Check your code for syntax errors, such as missing or misplaced tokens.";
        } else if (errorMessage.find("Expected") != std::string::npos) {
            return "Ensure the correct syntax is followed. " + errorMessage;
        } else if (errorMessage.find("Invalid value stack for unary operation")
                   != std::string::npos) {
            return "Ensure the stack has enough values for the operation.";
        } else if (errorMessage.find("Invalid value stack for binary operation")
                   != std::string::npos) {
            return "Ensure the stack has two values for the binary operation.";
        } else if (errorMessage.find("Unsupported type for NEGATE operation") != std::string::npos) {
            return "NEGATE operation only supports int32_t and double types.";
        } else if (errorMessage.find("Unsupported type for NOT operation") != std::string::npos) {
            return "NOT operation only supports bool type.";
        } else if (errorMessage.find("Division by zero") != std::string::npos) {
            return "Ensure the divisor is not zero.";
        } else if (errorMessage.find("Modulo by zero") != std::string::npos) {
            return "Ensure the divisor is not zero.";
        } else if (errorMessage.find("Unsupported types for binary operation")
                   != std::string::npos) {
            return "Binary operations support int32_t and double types.";
        } else if (errorMessage.find("Insufficient value stack for logical operation")
                   != std::string::npos) {
            return "Ensure the stack has two values for the logical operation.";
        } else if (errorMessage.find("Unsupported types for logical operation")
                   != std::string::npos) {
            return "Logical operations only support bool type.";
        } else if (errorMessage.find("Insufficient value stack for comparison operation")
                   != std::string::npos) {
            return "Ensure the stack has two values for the comparison operation.";
        } else if (errorMessage.find("Unsupported types for comparison operation")
                   != std::string::npos) {
            return "Comparison operations support int32_t and double types.";
        } else if (errorMessage.find("Invalid variable index") != std::string::npos) {
            return "Ensure the variable index is within the valid range.";
        } else if (errorMessage.find("value stack underflow") != std::string::npos) {
            return "Ensure there are enough values on the stack for the operation.";
        } else if (errorMessage.find("Invalid jump offset type") != std::string::npos) {
            return "Ensure the jump offset is of type int32_t.";
        } else if (errorMessage.find("JUMP_IF_FALSE requires a boolean condition")
                   != std::string::npos) {
            return "Ensure the condition for JUMP_IF_FALSE is a boolean.";
        } else {
            return "Check your code for errors.";
        }
    }

    static std::string stageToString(InterpretationStage stage)
    {
        switch (stage) {
        case InterpretationStage::SCANNING:
            return "Lexical Error";
        case InterpretationStage::PARSING:
            return "Syntax Error";
        case InterpretationStage::SYNTAX:
            return "Syntax Parsing";
        case InterpretationStage::SEMANTIC:
            return "Semantic Parsing";
        case InterpretationStage::INTERPRETING:
            return "Interpreting";
        case InterpretationStage::COMPILING:
            return "Compiling";
        default:
            return "Unknown stage";
        }
    }
};
