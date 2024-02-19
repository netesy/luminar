// debugger.h
#pragma once

#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

enum class InterpretationStage { SCANNING, PARSING, SYNTAX, SEMANTIC, INTERPRETING };

class Debugger {
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
        std::cerr << "Suggestion: " << getSuggestion(errorMessage) << std::endl;
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
        std::cerr << "Suggestion: " << getSuggestion(errorMessage)
                  << "\n ----------------END----------------\n"
                  << std::endl;
    }

private:
    static std::string getSuggestion(const std::string &errorMessage)
    {
        // Provide suggestions based on the error message
        if (errorMessage == "Invalid character.") {
            return "Check for invalid characters in your code.";
        } else if (errorMessage == "Variable/function not found") {
            return "Check the spelling of the variable or function name, or make sure it has been "
                   "declared or defined before use.";
        } else if (errorMessage == "Invalid factor") {
            return "Check the expression to ensure it follows the correct syntax.";
        } else {
            return "Check your code for errors.";
        }
    }
    static std::string stageToString(InterpretationStage stage)
    {
        switch (stage) {
        case InterpretationStage::SCANNING:
            return "Scanning";
        case InterpretationStage::PARSING:
            return "Parsing";
        case InterpretationStage::SYNTAX:
            return "Syntax Parsing";
        case InterpretationStage::SEMANTIC:
            return "Semantic Parsing";
        case InterpretationStage::INTERPRETING:
            return "Interpreting";
        default:
            return "Unknown stage";
        }
    }
};
