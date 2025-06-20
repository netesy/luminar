#include "repl.hh"
#include "parser/packrat.hh"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <algorithm>

REPL::REPL() {
    // Initialize core components
    memoryManager = std::make_unique<MemoryManager<>>();
    region = std::make_unique<MemoryManager<>::Region>(*memoryManager);
    typeSystem = std::make_shared<TypeSystem>(*memoryManager, *region);
    functions = std::make_unique<Functions>(typeSystem);
    
    // Initialize execution state
    resetExecutionState();
    
    std::cout << "Luminar REPL initialized" << std::endl;
}

REPL::~REPL() {
    cleanup();
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

void REPL::run(const std::string &input, const std::string &filename, const std::string &filepath) {
    if (input.empty()) {
        return;
    }

    try {
        // Create scanner and parser
        Scanner scanner(input, filename, filepath);
        parser = std::make_unique<PackratParser>(scanner, typeSystem, *functions);
        
        // Parse input
        parser->parse();
        
        // Get bytecode and create backend
        bytecode = parser->getBytecode();
        backend = std::make_unique<StackBackend>(bytecode, *functions, *memoryManager);
        
        // Create and run VM
        vm = std::make_unique<VM>(*parser, std::move(backend));
        vm->run();
        
        // Dump registers if in debug mode
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
    
    // Clear core components in reverse order of initialization
    vm.reset();
    backend.reset();
    parser.reset();
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
