#include <iostream>
#include <string>
#include <stdexcept>

#include "process.hpp"

#ifndef CHILD_PATH
    #define CHILD_PATH "./child"
#endif

void parent() {
    std::string file1{};
    if (!std::getline(std::cin, file1) || file1.empty()) {
        throw std::runtime_error("Failed to read filename");
    }

    std::string file2{};
    if (!std::getline(std::cin, file2) || file2.empty()) {
        throw std::runtime_error("Failed to read filename");
    }

    Process child1(CHILD_PATH, file1);
    Process child2(CHILD_PATH, file2);

    std::string line{};
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
