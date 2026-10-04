#include <iostream>
#include <string>

#include "string_utils.hpp"

void child() {
    std::string line{};

    while (std::getline(std::cin, line)) {
        reverse_string(line);
        std::cout << line << std::endl;
    }
}

int main() {
    child();

    return 0;
}
