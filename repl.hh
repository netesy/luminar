#pragma once

//#include "interpreter.h"
#include "scanner.hh"
#include "vm.hh"
#include <string>

class REPL
{
public:
    static void start();
    static void startDevMode();

private:
    static std::string readInput();
    static std::string readFile(const std::string& filename);
    static void debug(Scanner scanner, Parser parser);
};
