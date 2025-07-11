#include "repl.hh"
#include "parser/packrat.hh"
#include "parser/pratt.hh"
#include "scanner.hh"
#include "visitors/code_formatter.hh"
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

REPL::REPL() {
    // Initialize core components
    memoryManager = std::make_unique<MemoryManager<>>();
    region = std::make_unique<MemoryManager<>::Region>(*memoryManager);
    typeSystem = std::make_shared<TypeSystem>(*memoryManager, *region);
    functions = std::make_unique<Functions>(typeSystem);

    // Initialize execution state
    resetExecutionState();

    // Default to Pratt parser
    usePrattParser = true;

    std::cout << "Luminar REPL initialized" << std::endl;
}

REPL::~REPL() {
    cleanup();
}

std::string REPL::formatSourceCode(const std::string& source, const std::string& filename) {
    try {
        // Create scanner and parse the source
        Scanner scanner(source, filename, "");
        std::vector<std::unique_ptr<ASTNode>> astNodes;

        if (usePrattParser) {
            PrattParser prattParser(scanner, typeSystem);
            prattParser.parse();
            astNodes = prattParser.getAST();
        }

        // if (!astNodes.empty()) {
        //     // For now, just format the first node
        //     // In the future, we might want to format all nodes
        //     auto formatter = std::make_unique<CodeFormatter>(typeSystem);
        //     return formatter->format(*astNodes[0]);
        // }
    } catch (const std::exception& e) {
        std::cerr << "Formatting error: " << e.what() << std::endl;
    }

    // Fallback to simple formatting
    std::istringstream input(source);
    std::string line;
    std::string formattedCode;

    while (std::getline(input, line)) {
        auto start = line.find_first_not_of(" \t");
        if (start != std::string::npos) {
            auto end = line.find_last_not_of(" \t");
            line = line.substr(start, end - start + 1);

            if (!formattedCode.empty()) {
                formattedCode += "\n";
            }
            formattedCode += line;
        }
    }

    return formattedCode.empty() ? source : formattedCode;
}

void REPL::start(const std::string &filename) {
    std::cout << "Luminar Interpreter" << std::endl;

    if (!filename.empty()) {
        try {
            std::string fileContent = readFile(filename);
            std::string filePath = std::filesystem::absolute(filename).string();
            if (!fileContent.empty()) {
                std::cout << "Interpreting file: " << filename << std::endl;
                run(fileContent, filename, filePath);
            } else {
                std::cerr << "Error: File is empty: " << filename << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    // Start the REPL loop
    std::string input;
    while (true) {
        try {
            input = readInput();
            if (input == "exit" || input == "quit") {
                break;
            } else if (input == "debug") {
                if (parser) {
                    std::cout << "Debug info available. Use 'dump' command in the future." << std::endl;
                } else {
                    std::cout << "No active parser state to debug." << std::endl;
                }
                continue;
            } else if (input == "clear") {
                resetExecutionState();
                std::cout << "Execution state cleared." << std::endl;
                continue;
            } else if (input == "parser pratt") {
                usePrattParser = true;
                std::cout << "Switched to Pratt parser" << std::endl;
                continue;
            } else if (input == "parser packrat") {
                usePrattParser = false;
                std::cout << "Switched to Packrat parser" << std::endl;
                continue;
            } else if (input == "parser status") {
                std::cout << "Current parser: " << (usePrattParser ? "Pratt" : "Packrat") << std::endl;
                continue;
            } else if (input.substr(0, 7) == "format ") {
                std::string code = input.substr(7);
                std::string formatted = formatSourceCode(code, "<format>");
                std::cout << "Formatted code:\n" << formatted << std::endl;
                continue;
            }

            run(input);
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            resetExecutionState();
        }
    }
}

void REPL::startDevMode(const std::string &filename) {
    std::cout << "Luminar Dev Mode" << std::endl;
    start(filename);
}

void REPL::parseInput(const std::string &input, const std::string &filename, const std::string &filepath) {
    Scanner scanner(input, filename, filepath);

    if (usePrattParser) {
        try {
            PrattParser prattParser(scanner, typeSystem);
            prattParser.parse();
            bytecode = prattParser.getBytecode();
        } catch (const std::exception& e) {
            std::cerr << "Pratt parser error: " << e.what() << std::endl;
            throw;
        }
    } else {
        try {
            parser = std::make_unique<PackratParser>(scanner, typeSystem, *functions);
            parser->parse();
            bytecode = parser->getBytecode();
        } catch (const std::exception& e) {
            std::cerr << "Packrat parser error: " << e.what() << std::endl;
            throw;
        }
    }
}

void REPL::run(const std::string &input, const std::string &filename, const std::string &filepath) {
    if (input.empty()) {
        return;
    }

    try {
        // Parse input using selected parser
        parseInput(input, filename, filepath);

        // Create backend and run VM
        backend = std::make_unique<StackBackend>(bytecode, *functions, *memoryManager);
        vm = std::make_unique<VM>(*parser, std::move(backend));
        vm->run();

        // Clean up and dump registers if in debug mode
        if (vm) {
            vm->dumpRegisters();
        }
    } catch (const std::exception &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        throw;
    }
}

std::string REPL::readInput() const {
    std::string input;
    std::cout << ">> ";
    std::getline(std::cin, input);

    // Trim whitespace
    input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
    input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);

    return input;
}

std::string REPL::readFile(const std::string &filename) const {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void REPL::debug(const Scanner &scanner, const Algorithm &parser) const {
    std::ofstream debugfile("debug.log", std::ios::app);
    if (!debugfile.is_open()) {
        std::cerr << "Failed to open debug log file" << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    debugfile << "\n=== Debug Session " << std::ctime(&time) << "===\n";
    debugfile << "Scanner state:\n" << scanner.toString() << "\n";
    debugfile << "Parser state:\n" << parser.toString() << "\n\n";
    debugfile.close();
}

void REPL::initializeVM() {
    if (parser) {
        bytecode = parser->getBytecode();
        backend = std::make_unique<StackBackend>(bytecode, *functions, *memoryManager);
        vm = std::make_unique<VM>(*parser, std::move(backend));
    }
}

void REPL::cleanup() {
    // Clear execution state
    resetExecutionState();

    // Clean up in reverse order of initialization
    if (vm) {
        // vm->cleanup();
        vm.reset();
    }
    backend.reset();
    parser.reset();
    bytecode.clear();
    functions.reset();
    typeSystem.reset();
    region.reset();
    memoryManager.reset();
}

void REPL::resetExecutionState() {
    // Clear execution state
    bytecode.clear();

    // Reset VM and backend
    vm.reset();
    backend.reset();

    // Clear parser state
    parser.reset();
}
