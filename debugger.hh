// debugger.h
#pragma once

#include "token.hh"
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
public:
    static void error(const std::string &errorMessage,
                      const Token &errorToken,
                      InterpretationStage stage,
                      const std::string &code,
                      const std::string &expectedValue = "");

private:
    static std::vector<std::string> sourceCodez;

    static void debugConsole(const std::string &errorMessage,
                             const Token &errorToken,
                             InterpretationStage stage,
                             const std::string &expectedValue);
    static void debugLog(const std::string &errorMessage,
                         const Token &errorToken,
                         InterpretationStage stage,
                         const std::string &expectedValue);
    static void printContextLines(std::ostream &out, int errorLine);
    static std::vector<std::string> splitLines(const std::string &sourceCode);
    static std::string getTime();
    static std::string getSuggestion(const std::string &errorMessage,
                                     const std::string &expectedValue);
    static std::string getSampleSolution(const std::string &errorMessage,
                                         const std::string &expectedValue);
    static std::string stageToString(InterpretationStage stage);
};
