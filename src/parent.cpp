#include <iostream>
#include <string>
#include <stdexcept>

#include "process.hpp"

#ifndef CHILD_PATH
    #define CHILD_PATH "./child"
#endif

#if defined(_WIN32)
    #include <windows.h>
    inline unsigned long get_current_pid() {
        return static_cast<unsigned long>(GetCurrentProcessId());
    }
#else
    #include <unistd.h>
    inline unsigned long get_current_pid() {
        return static_cast<unsigned long>(getpid());
    }
#endif

void parent() {
    std::cerr << "[Parent] PID: " << get_current_pid() << '\n';

    std::cout << "Enter output file for Child1: ";
    std::string file1{};
    if (!std::getline(std::cin, file1) || file1.empty()) {
        throw std::runtime_error("Failed to read filename");
    }

    std::cout << "Enter output file for Child2: ";
    std::string file2{};
    if (!std::getline(std::cin, file2) || file2.empty()) {
        throw std::runtime_error("Failed to read filename");
    }

    Process child1(CHILD_PATH, file1);
    Process child2(CHILD_PATH, file2);

    std::cerr << "[Parent] Created Child1 (PID: " << child1.get_pid() << ")\n";
    std::cerr << "[Parent] Created Child2 (PID: " << child2.get_pid() << ")\n";

    std::string line{};
    
    std::cout << "Enter lines for processing (Ctrl+D / Ctrl+C to finish): \n";
    while (std::getline(std::cin, line)) {
        if (line.length() > 10) {
            child2.send_line(line);
        } else {
            child1.send_line(line);
        }
    }

    child1.close_input();
    child2.close_input();

    child1.wait();
    child2.wait();

    std::cerr << "[Parent] Closed Child1\n";
    std::cerr << "[Parent] Closed Child2\n";
}

int main() {
    try {
        parent();
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;

    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return 1;
    }
}
