#include "FileSystem.h"

int main() {
    setlocale(LC_ALL, "");
    FileSystem fs;
    std::string user_input;
    while (true) {
        std::cout << fs.getCurrentFolder()->getPath() << '>';
        std::getline(std::cin, user_input);
        if (user_input == "exit") break;
        fs.CommandProcessor(user_input);
    }
    return 0;
}