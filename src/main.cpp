#include "parser/pratt.hh"
#include "repl.hh"
#include "tutorial.hh"
#include "visitors/code_formatter.hh"
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct CLIConfig {
    std::string mode;
    std::string target;
    std::string sourceFile;
    bool devMode = false;
    bool formatCode = false;
    std::string outputPath;
    bool tutorialMode = false;
};

class CLIManager {
private:
    struct Command {
        std::string name;
        std::string description;
        std::function<void(REPL&, CLIConfig&)> handler;
    };

    static std::vector<Command> commands;

    static bool startsWith(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }

    static void handleBuild(REPL& repl, CLIConfig& config) {
        if (config.target.empty()) {
            std::cout << "Enter target (windows/mac/linux): ";
            std::cin >> config.target;
        }
        std::cout << "\033[1;33mBuilding for " << config.target << "...\033[0m\n";
        // Build logic would go here
    }

    static void handleRun(REPL& repl, CLIConfig& config) {
        if (config.sourceFile.empty()) {
            std::cout << "Enter source file to run (or press Enter for REPL): ";
            std::cin.ignore();
            std::getline(std::cin, config.sourceFile);
        }
        if (!config.sourceFile.empty()) {
            std::cout << "\033[1;32mRunning " << config.sourceFile << "...\033[0m\n";
            config.devMode ? repl.startDevMode(config.sourceFile) : repl.start(config.sourceFile);
        } else {
            std::cout << "\033[1;32mStarting REPL...\033[0m\n";
            repl.startDevMode("");
        }
    }

    static void handleRepl(REPL& repl, CLIConfig& config) {
        std::cout << "\033[1;32mStarting REPL...\033[0m\n";
        config.devMode ? repl.startDevMode("") : repl.start("");
    }

    static void handleNew(REPL&, CLIConfig&) {
        std::cout << "\033[1;32mSetting up new Luminar project...\033[0m\n";
        // Project setup logic
    }

    static void handleInit(REPL&, CLIConfig&) {
        std::cout << "\033[1;32mInitializing Luminar project...\033[0m\n";
        // Project initialization logic
    }


    static void handleFormat(REPL& repl, CLIConfig& config) {
        if (config.sourceFile.empty()) {
            std::cout << "Enter source file to format: ";
            std::cin.ignore();
            std::getline(std::cin, config.sourceFile);
        }

        try {
            // Read the source file
            std::ifstream inputFile(config.sourceFile);
            if (!inputFile) {
                std::cerr << "Error: Could not open input file: " << config.sourceFile << std::endl;
                return;
            }

            std::string source((std::istreambuf_iterator<char>(inputFile)),
                             std::istreambuf_iterator<char>());
            inputFile.close();

            // Format the code using REPL's method
            std::string formattedCode = repl.formatSourceCode(source, config.sourceFile);

            // Create a backup of the original file
            std::string backupFile = config.sourceFile + ".bak";
            std::filesystem::copy_file(config.sourceFile, backupFile, 
                                     std::filesystem::copy_options::overwrite_existing);
            
            // Write the formatted code back to the original file
            std::ofstream outputFile(config.sourceFile);
            if (!outputFile) {
                std::cerr << "Error: Could not write to file: " << config.sourceFile << std::endl;
                return;
            }

            outputFile << formattedCode;
            outputFile.close();
            
            std::cout << "Successfully formatted " << config.sourceFile << std::endl;
            std::cout << "Original saved as: " << backupFile << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error formatting file: " << e.what() << std::endl;
            throw;
        }
    }

    static void handleHelp(REPL&, CLIConfig&) {
        showLogo();
        showHelp("luminar");
    }

    static void handleZen(REPL&, CLIConfig&) { showZenOfLuminar(); }

    static void handleTutorial(REPL& repl, CLIConfig&) {
        std::cout << "\033[1;32mWelcome to the Luminar Tutorial!\033[0m\n";
        showZenOfLuminar();

        Tutorial tutorial;

        while (true) {
            std::cout << "\n\033[1;34mTutorial Sections:\033[0m\n";
            for (size_t i = 0; i < tutorial.sections.size(); i++) {
                std::cout << i + 1 << ". " << tutorial.sections[i].title << "\n";
            }
            std::cout << tutorial.sections.size() + 1 << ". Exit Tutorial\n\n";

            size_t choice;
            std::cout << "Select a section (1-" << tutorial.sections.size() + 1 << "): ";
            std::cin >> choice;

            if (choice == tutorial.sections.size() + 1) {
                break;
            }

            if (choice > 0 && choice <= tutorial.sections.size()) {
                const auto& section = tutorial.sections[choice - 1];
                std::cout << "\n\033[1;33m" << section.title << "\033[0m\n";
                std::cout << section.description << "\n\nExamples:\n";
                for (const auto& example : section.examples) {
                    std::cout << "\033[1;32m" << example << "\033[0m\n";
                    std::cout << "Try it out (press Enter to continue):\n";
                    std::cin.ignore();
                    std::string input;
                    std::getline(std::cin, input);
                    if (!input.empty()) {
                        repl.run(input, "", "");
                    }
                }
            }
        }
    }

    static void handleInvalid(REPL&, CLIConfig&) {
        std::cerr << "\033[1;31mError: Invalid command.\033[0m\n";
        showHelp("luminar");
    }

public:
    static void showZenOfLuminar() {
        std::cout << "\033[1;36mThe Zen of Luminar:\033[0m\n"
                  << "1. Readability Counts: Code should be easy to read and understand.\n"
                  << "2. Efficiency Matters: Optimize for performance without sacrificing clarity.\n"
                  << "3. Simplicity is Key: Prefer simple solutions over complex ones.\n"
                  << "4. Expressiveness: Provide powerful language features without unnecessary verbosity.\n"
                  << "5. Flexibility: Support both low-level and high-level programming paradigms.\n"
                  << "6. Safety with Power: Enable advanced features while ensuring safe usage.\n"
                  << "7. Consistency: Follow uniform conventions and patterns throughout the language.\n"
                  << "8. Interoperability: Allow seamless integration with other languages and systems.\n"
                  << "9. Modularity: Encourage splitting code into reusable modules.\n"
                  << "10. Error Handling: Make error detection and handling straightforward and explicit.\n"
                  << "11. Scalability: Support small scripts to large, complex systems with ease.\n"
                  << "12. Concurrency: Facilitate concurrent and parallel programming naturally.\n"
                  << "13. Memory Management: Balance automatic and manual memory management efficiently.\n"
                  << "14. Documentation: Ensure every feature is well-documented and discoverable.\n"
                  << "15. Evolve Gracefully: Allow the language to grow without breaking existing code.\n"
                  << "16. Community: Foster an open and welcoming community for contributors.\n"
                  << "17. Tooling: Provide robust tools for development, debugging, and deployment.\n";
    }

    static void showLogo() {
        std::cout << "\033[1;33m"
                     "##      ##    ##  ###    ###  ##  ###    ##  #####   #####   \n"
                     "##      ##    ##  ####  ####  ##  ####   ##  ##  ##  ##  ##  \n"
                     "##      ##    ##  ## #### ##  ##  ## ##  ##  ######  #####   \n"
                     "##      ##    ##  ##  ##  ##  ##  ##  ## ##  ##  ##  ##  ##  \n"
                     "######   ######   ##      ##  ##  ##   ####  ##  ##  ##  ##  \n"
                     "######    ####    ##      ##  ##  ##    ###  ##  ##  ##  ##  \n"
                     "         Luminar Interpreter - Version 0.0.1              \n"
                     "\033[0m";
    }

    static void showHelp(const char* programName) {
        std::cout << "\033[1;34mUsage:\033[0m " << programName << " [command] [options]\n\n"
                  << "\033[1;36mCommands:\033[0m\n";

        for (const auto& cmd : commands) {
            std::cout << "  \033[1;32m" << cmd.name << "\033[0m - " << cmd.description << "\n";
        }
        std::cout << "\n\033[1;36mBuild Targets:\033[0m\n"
                  << "  \033[1;33mwindows\033[0m          Build for Windows\n"
                  << "  \033[1;33mmac\033[0m              Build for macOS\n"
                  << "  \033[1;33mlinux\033[0m            Build for Linux \n";

        std::cout << "\n\033[1;36mOptions:\033[0m\n"
                  << "  \033[1;35m--dev\033[0m         Enable development mode\n"
                  << "  \033[1;35m--format\033[0m      Enable code formatting\n"
                  << "  \033[1;35m--output=<path>\033[0m Set output path\n"
                  << "  \033[1;35m-h, --help\033[0m    Show this help message\n";

        std::cout << "\033[1;36mExamples:\033[0m\n"
                  << "  " << programName << " \033[1;32mrun example.lm\033[0m\n"
                  << "  " << programName << " \033[1;32mbuild linux\033[0m\n"
                  << "  " << programName << " \033[1;32mrepl --dev\033[0m\n";
    }

    static void parseCommandLine(REPL& repl, int argc, char* argv[], CLIConfig& config) {
        std::string command;

        if (argc > 1) {
            command = argv[1];
        } else {
            showLogo();
            showHelp("luminar");
            std::cout << "\033[1;33mEnter a command (or type 'help'): \033[0m";
            std::getline(std::cin, command);
        }

        if (command.empty()) {
            std::cerr << "\033[1;31mNo command provided. Type 'help' for available commands.\033[0m\n";
            return;
        }

        config.mode = command;

        // Process additional arguments
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--dev") config.devMode = true;
            else if (arg == "--format") config.formatCode = true;
            else if (startsWith(arg, "--output=")) config.outputPath = arg.substr(9);
            else if (config.mode == "build") config.target = arg;
            else if (config.mode == "run") config.sourceFile = arg;
        }

        // Find and execute the command
        auto it = std::find_if(commands.begin(), commands.end(),
            [&command](const Command& cmd) { return cmd.name == command; });

        if (it != commands.end()) {
            it->handler(repl, config);
        } else {
            handleInvalid(repl, config);
        }
    }
};

// Initialize static member
std::vector<CLIManager::Command> CLIManager::commands = {
    {"build", "Build for a specific target", handleBuild},
    {"run", "Run a source file or start the REPL", handleRun},
    {"repl", "Start REPL mode", handleRepl},
    {"format", "Format a source file", handleFormat},
    {"help", "Show this help message", handleHelp},
    {"new", "Create a new Luminar project", handleNew},
    {"init", "Setup a luminar project in current folder", handleInit},
    {"zen", "Show the Zen of Luminar", handleZen},
    {"tutorial", "Start the tutorial", handleTutorial}
};

int main(int argc, char* argv[]) {
    CLIConfig config;
    REPL repl;
    CLIManager::parseCommandLine(repl, argc, argv, config);
    return 0;
}
