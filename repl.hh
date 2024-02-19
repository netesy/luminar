#pragma once

//#include "interpreter.h"
#include <string>

class REPL
{
public:
    static void start();
    static void startDevMode();

private:
    static std::string readInput();
    static std::string readFile(const std::string& filename);
};
