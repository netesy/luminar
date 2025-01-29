#include "repl.hh"
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <filesystem>

struct CLIConfig
{
    std::string mode;
    std::string target;
    std::string sourceFile;
    bool devMode = false;
    bool formatCode = false;
    std::string outputPath;
};

class CLIManager
{
private:
    using CommandHandler = std::function<void(CLIConfig &)>;
    static std::map<std::string, CommandHandler> commandHandlers;
    static bool startsWith(const std::string &str, const std::string &prefix)
    {
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }
    static void handleBuild(CLIConfig &config)
    {
        if (config.target.empty()) {
            std::cout << "Enter target (windows/mac/linux): ";
            std::cin >> config.target;
        }
        std::cout << "\033[1;33mBuilding for " << config.target << "...\033[0m\n";
        REPL::start("");
    }

    static void handleRun(CLIConfig &config)
    {
        if (config.sourceFile.empty()) {
            std::cout << "Enter source file to run (or press Enter for REPL): ";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::getline(std::cin, config.sourceFile);
        }
        if (!config.sourceFile.empty()) {
            std::cout << "\033[1;32mRunning " << config.sourceFile << "...\033[0m\n";
            config.devMode ? REPL::startDevMode(config.sourceFile) : REPL::start(config.sourceFile);
        } else {
            std::cout << "\033[1;32mStarting REPL...\033[0m\n";
            REPL::startDevMode("");
        }
    }

    static void handleRepl(CLIConfig &config)
    {
        std::cout << "\033[1;32mStarting REPL...\033[0m\n";
        config.devMode ? REPL::startDevMode("") : REPL::start("");
    }

    static void handleFormat(CLIConfig &config)
    {
        if (config.sourceFile.empty()) {
            std::cout << "Enter source file to format: ";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::getline(std::cin, config.sourceFile);
        }
        std::cout << "\033[1;32mFormatting " << config.sourceFile << "...\033[0m\n";
        // Add formatting logic here
    }

    static void handleNew(CLIConfig &)
    {
        std::cout << "\033[1;32mSetting up new Luminar project...\033[0m\n";
        // Add project setup logic here
    }

    static void handleInit(CLIConfig &)
    {
        std::cout << "\033[1;32mInitializing Luminar project...\033[0m\n";
        // Add project initialization logic here
    }

    static void handleHelp(CLIConfig &)
    {
        showLogo();
        showHelp("luminar");
    }

    static void handleInvalid(CLIConfig &)
    {
        std::cerr << "\033[1;31mError: Invalid command.\033[0m\n";
        showHelp("luminar");
    }

    static void promptForOptions(CLIConfig &config)
    {
        std::cout << "\033[1;34mOptions:\033[0m\n"
                  << "1. Enable development mode (--dev)\n"
                  << "2. Enable code formatting (--format)\n"
                  << "3. Set output path (--output)\n"
                  << "4. Done\n";
        int option;
        while (true) {
            std::cout << "Enter your choice (1-4): ";
            std::cin >> option;
            if (option == 4)
                break;

            switch (option) {
            case 1:
                config.devMode = true;
                std::cout << "Development mode enabled\n";
                break;
            case 2:
                config.formatCode = true;
                std::cout << "Code formatting enabled\n";
                break;
            case 3:
                std::cout << "Enter output path: ";
                std::cin.ignore();
                std::getline(std::cin, config.outputPath);
                std::cout << "Output path set to: " << config.outputPath << "\n";
                break;
            default:
                std::cerr << "\033[1;31mInvalid option.\033[0m\n";
                break;
            }
        }
    }

public:
    static void showLogo()
    {
            std::cout << "\033[1;33m"
                         "##      ##    ##  ###    ###  ##  ###    ##  #####  #####   \n"
                         "##      ##    ##  ####  ####  ##  ####   ##  ##  ##  ##  ##  \n"
                         "##      ##    ##  ## #### ##  ##  ## ##  ##  ######  #####   \n"
                         "##      ##    ##  ##  ##  ##  ##  ##  ## ##  ##  ##  ##  ##  \n"
                         "######   ######   ##      ##  ##  ##   ####  ##  ##  ##  ##  \n"
                         "######    ####    ##      ##  ##  ##    ###  ##  ##  ##  ##  \n"
                         "         Luminar Interpreter - Version 0.0.1              \n"
                         "\033[0m";
    }

    static void showHelp(const char *programName)
    {
        std::cout << "\033[1;34mUsage:\033[0m " << programName << " [command] [options]\n\n"
                  << "\033[1;36mCommands:\033[0m\n"
                  << "  \033[1;32mbuild <target>\033[0m     Build for specific target\n"
                  << "  \033[1;32mrun [file]\033[0m       Run source file or REPL\n"
                  << "  \033[1;32mrepl\033[0m             Start REPL mode\n"
                  << "  \033[1;32mformat <file>\033[0m    Format source code\n"
                  << "  \033[1;32mnew\033[0m              Setup file structure for a new Luminar "
                     "project\n"
                  << "  \033[1;32minit\033[0m             Initialize an existing Luminar project\n"
                  << "  \033[1;32mhelp\033[0m             Show this help message\n\n"
                  << " \n"
                  << "\033[1;36mBuild Targets:\033[0m\n"
                  << "  \033[1;33mwindows\033[0m          Build for Windows\n"
                  << "  \033[1;33mmac\033[0m              Build for macOS\n"
                  << "  \033[1;33mlinux\033[0m            Build for Linux\n\n"
                  << " \n"
                  << "\033[1;36mOptions:\033[0m\n"
                  << "  \033[1;35m--dev\033[0m           Enable development mode\n"
                  << "  \033[1;35m--format\033[0m        Enable code formatting\n"
                  << "  \033[1;35m--output=<path>\033[0m Set output path\n"
                  << "  \033[1;35m-h, --help\033[0m      Show this help message\n"
                  << " \n"
                  << "\033[1;36mExamples:\033[0m\n"
                  << "  " << programName << " \033[1;32mrun example.lm\033[0m\n"
                  << "  " << programName << " \033[1;32mbuild linux\033[0m\n"
                  << "  " << programName << " \033[1;32mrepl --dev\033[0m\n";
    }

    static void parseCommandLine(int argc, char *argv[], CLIConfig &config) {
        std::string command;

        if (argc < 2) {
            showLogo();
            // std::cout << "\033[1;34mEnter a command (or 'help' for usage info):\033[0m\n";
            // std::getline(std::cin, command);
            showHelp("luminar");
            REPL::startDevMode("");

            // Handle empty input by showing help and starting REPL
            if (command.empty()) {
                showHelp(argv[0]);
                REPL::startDevMode("");
                return;
            }
        } else {
            command = argv[1];
        }

        config.mode = command;

        // Process additional arguments if they exist from command line
        if (argc > 2) {
            for (int i = 2; i < argc; i++) {
                std::string arg = argv[i];
                processArgument(arg, config);
            }
        }
        // If no additional arguments but command needs them, prompt user
        else {
            promptForArguments(command, config);
        }

        // Execute the command
        auto handler = commandHandlers.find(command);
        if (handler != commandHandlers.end()) {
            handler->second(config);
        } else {
            handleInvalid(config);
        }
    }

    // Helper function to process individual arguments
    static void processArgument(const std::string &arg, CLIConfig &config) {
        if (arg == "--dev") {
            config.devMode = true;
        }
        else if (arg == "--format") {
            config.formatCode = true;
        }
        else if (startsWith(arg, "--output=")) {
            config.outputPath = arg.substr(9);
        }
        else if (config.mode == "build") {
            config.target = arg;
        }
        else if (config.mode == "run") {
            config.sourceFile = arg;
        }
    }

    // Helper function to prompt for missing arguments based on command
    static void promptForArguments(const std::string &command, CLIConfig &config) {
        if (command == "build" && config.target.empty()) {
            std::cout << "Enter target (windows/mac/linux): ";
            std::getline(std::cin, config.target);
        }
        else if (command == "run" && config.sourceFile.empty()) {
            std::cout << "Enter source file to run (or press Enter for REPL): ";
            std::getline(std::cin, config.sourceFile);
        }
        else if (command == "format" && config.sourceFile.empty()) {
            std::cout << "Enter source file to format: ";
            std::getline(std::cin, config.sourceFile);
        }

        // Always prompt for additional options
        std::cout << "\nWould you like to set additional options? (y/n): ";
        std::string response;
        std::getline(std::cin, response);

        if (response == "y" || response == "Y") {
            promptForOptions(config);
        }
    }
};

std::map<std::string, CLIManager::CommandHandler> CLIManager::commandHandlers = {
    {"build", handleBuild},
    {"run", handleRun},
    {"repl", handleRepl},
    {"format", handleFormat},
    {"new", handleNew},
    {"init", handleInit},
    {"help", handleHelp}
};

int main(int argc, char *argv[]) {
    CLIConfig config;
    CLIManager::parseCommandLine(argc, argv, config);
    return 0;
}
