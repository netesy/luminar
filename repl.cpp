#include "repl.hh"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <variant>

REPL::REPL()
    : backend(std::make_unique<StackBackend>(bytecode))
    , vm(nullptr)
{}

void REPL::start()
{
    std::cout << "Luminar REPL :" << std::endl;
    std::string input = readFile("test.lm");
    run(input, "test.lm", std::filesystem::absolute("test.lm").string());
}

void REPL::run(std::string input, const std::string &filename = "", const std::string &filepath = "")
{
        // Tokenize input
    Scanner scanner(input, filename, filepath);
    Parser parser(scanner);
    debug(scanner, parser);
    std::vector<Instruction> bytecode = parser.getBytecode();
    auto backend = std::make_unique<StackBackend>(bytecode); // passing by value
    VM vm(parser, std::move(backend));

    try {
        vm.run();
        vm.dumpRegisters();

            } catch (const std::exception &e) {
                std::cerr << " Repl Error: " << e.what() << std::endl;
                 // Debugger::error(e.what(), 0, 0, "", Debugger::getSuggestion(e.what()));
        }
        //    }
}

void REPL::startDevMode()
{
    //start the language with debugging enabled.
    std::cout << "Luminar Dev REPL :" << std::endl;
    std::string input = readInput();
    run(input);
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
