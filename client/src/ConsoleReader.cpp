#include "ConsoleReader.hpp"

#include <iostream>

#include <QString>

void ConsoleReader::run() {
    std::string input;
    while (true) {
        // The thread wait for user input
        if (std::getline(std::cin, input)) {
            QString msg = QString::fromStdString(input);

            emit messageTyped(msg);
        }
    }
}
