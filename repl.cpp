#include "repl.hh"
#include <fstream>
#include <iostream>
#include <string>
#include <variant>

void REPL::start()
{
    std::cout << "Luminar REPL :" << std::endl;
    while (true) {
        // Read input
        std::string input = readInput();
        // Tokenize input
        Scanner scanner(input);
        Parser parser(scanner);
        RegisterVM vm(parser);
        try {
            vm.run();
            vm.dumpRegisters();

        } catch (const std::exception &e) {
            std::cerr << " Repl Error: " << e.what() << std::endl;
            // Debugger::error(e.what(), 0, 0, "", Debugger::getSuggestion(e.what()));
        }
    }
}

void REPL::startDevMode()
{
    //start the language with debugging enabled.
    std::cout << "Not yet Implemented" << std::endl;
    std::cout << "Luminar Dev REPL :" << std::endl;
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

void REPL::debug(Scanner scanner, Parser parser)
{
    //Now parse the tokens using scanner methods directly
    std::cout << "======= Scanner Debug =======\n"
              << scanner.toString() << "======= End Debug =======\n"
              << std::endl;

    std::cout << "======= parse Debug =======\n"
              << parser.toString() << "======= End Debug =======\n"
              << std::endl;
}
