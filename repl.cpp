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

void REPL::start(const std::string &filename = "test.lm")
{
    std::cout << "Luminar Interpreter :" << std::endl;
    if (!filename.empty()) {
        // If a filename is provided, read and run the file
        std::string fileContent = readFile(filename);
        std::string filePath = std::filesystem::absolute(filename).string();
        if (!fileContent.empty()) {
            std::cout << "Interpreting file: " << filename << std::endl;
            run(fileContent, filename, filePath);
        } else {
            std::cerr << "Error: Unable to read file or file is empty." << std::endl;
        }
    }

    // Start the REPL loop
    while (true) {
        std::string input = readInput();
        if (input == "exit" || input == "quit") {
            break;
        }
        run(input, "", "");
    }
    //    std::string input = readFile("test.lm");
    //    run(input, "loop.lm", std::filesystem::absolute("loop.lm").string());
}

void REPL::run(std::string input, const std::string &filename = "", const std::string &filepath = "")
{
        // Tokenize input
    Scanner scanner(input, filename, filepath);
    std::shared_ptr<TypeSystem> typeSystem = std::make_shared<TypeSystem>();
    Parser parser(scanner, typeSystem);
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
}

void REPL::startDevMode(const std::string &filename = "")
{
    std::cout << "Luminar Dev REPL :" << std::endl;
    if (!filename.empty()) {
        // If a filename is provided, read and run the file
        std::string fileContent = readFile(filename);
        std::string filePath = std::filesystem::absolute(filename).string();
        if (!fileContent.empty()) {
            std::cout << "Interpreting file: " << filename << std::endl;
            run(fileContent, filename, filePath);
        } else {
            std::cerr << "Error: Unable to read file or file is empty." << std::endl;
        }
    }

    // Start the REPL loop
    while (true) {
        std::string input = readInput();
        if (input == "exit" || input == "quit") {
            break;
        }
        run(input);
    }
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
    std::ofstream debugfile("debug_file.log",
                            std::ios_base::app); // Open debug log file for appending
    if (!debugfile.is_open()) {
        std::cerr << "Failed to open debug log file." << std::endl;
        return;
    }
    //Now parse the tokens using scanner methods directly
    debugfile << "======= Scanner Debug =======\n"
              << scanner.toString() << "======= End Debug =======\n"
              << std::endl;

    debugfile << "======= parse Debug =======\n"
              << parser.toString() << "======= End Debug =======\n"
              << std::endl;
    debugfile.close(); // Close the log file
}
