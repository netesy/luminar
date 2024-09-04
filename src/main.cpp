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
                    REPL::start("");
                } else if (target == "mac") {
                    // Call function to build for macOS
                    // buildMac();
                    std::cout << "Building for macOS..." << std::endl;
                    REPL::start("");
                } else if (target == "linux") {
                    // Call function to build for Linux
                    // buildLinux();
                    REPL::start("");
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
            if (argc > 2) {
                std::string filename = argv[2];
                // Start the REPL in development mode with hot reloading and interpret the specified file
                REPL::startDevMode(filename);
            } else {
                // Start the REPL in development mode with hot reloading without a specific file
                REPL::startDevMode("");
            }
        } else if (mode == "help" || "-h") {
            std::cout
                << "Usage: " << argv[0] << " [mode] [options]\n"
                << "Modes:\n"
                << "  build <target>   Build for the specified target (windows, mac, linux)\n"
                << "  run [file]       Start the REPL in development mode with hot reloading\n"
                << "                   Optionally specify a file to interpret\n"
                << "  help, -h         Display this help message\n"
                << std::endl;
        } else {
            std::cerr << "Error: Invalid mode. Use 'build', 'run', or 'help'." << std::endl;
            return 1;
        }
    } else {
        // Otherwise, start the REPL in interactive mode
        // REPL::start();
        REPL::startDevMode("");
    }

    return 0;
}
