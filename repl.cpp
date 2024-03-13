#include "repl.hh"
#include "parser.hh"
#include "scanner.hh"
#include "vm.hh" // Include the VM header
#include <fstream>
#include <iostream>
#include <string>
#include <variant>

void REPL::start()
{
    std::cout << "Luminar REPL :" << std::endl;
    while (true) {
        try {
            // Read input
            std::string input = readInput();
            // Parse input
            Scanner scanner(input);
            Parser parser(scanner);
            parser.parse();
            std::cout << "======= parse Debug =======\n"
                      << parser.toString() << "======= End Debug =======\n"
                      << std::endl;
            //print("parse function");
            std::vector<Instruction> bytecode = parser.getBytecode();

            // Create a Virtual Machine (VM) instance
            RegisterVM vm;

            // Evaluate the bytecode using the VM
            vm.Run(bytecode);

        } catch (const std::exception &e) {
            // std::cout << e << std::endl;
            // Debugger::error(e.what(), 0, 0, "", Debugger::getSuggestion(e.what()));
        }
    }
}

void REPL::startDevMode()
{
    //start the language with debugging enabled.
    std::cout << "Not yet Implemented" << std::endl;
    start();
}

std::string REPL::readInput()
{
    std::cout << "$ ";
    std::string input;
    std::getline(std::cin, input);
    return input;
}

std::string REPL::readFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}
