#include "repl.hh"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc > 1) {
        // If command-line arguments are provided, check for build or run mode
        std::string mode = argv[1];

        if (mode == "build") {
            // Check for the target operating system
            if (argc > 2) {
                std::string target = argv[2];
                if (target == "windows") {
                    // Call function to build for Windows
                    // buildWindows();
                    std::cout << "Building for Windows..." << std::endl;
                    REPL::start();
                } else if (target == "mac") {
                    // Call function to build for macOS
                    // buildMac();
                    std::cout << "Building for macOS..." << std::endl;
                    REPL::start();
                } else if (target == "linux") {
                    // Call function to build for Linux
                    // buildLinux();
                    REPL::start();
                    std::cout << "Building for Linux..." << std::endl;
                } else {
                    std::cerr << "Error: Invalid target operating system." << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: Missing target operating system." << std::endl;
                return 1;
            }
        } else if (mode == "run") {
            // Start the REPL in development mode with hot reloading
            REPL::startDevMode();
        } else {
            std::cerr << "Error: Invalid mode. Use 'build' or 'run'." << std::endl;
            return 1;
        }
    } else {
        // Otherwise, start the REPL in interactive mode
        // REPL::start();
        REPL::startDevMode();
    }

    return 0;
}
