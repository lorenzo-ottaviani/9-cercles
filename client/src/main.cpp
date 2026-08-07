#include "ChatClient.hpp"
#include "ConsoleReader.hpp"

#include <iostream>
#include <string>

#include <QCoreApplication>
#include <QObject>
#include <QString>

int main(int argc, char *argv[]) {
    // Start Qt console application
    QCoreApplication app(argc, argv);

    // Choose the username
    std::string raw_username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, raw_username);

    // Convert to QString
    QString username = QString::fromStdString(raw_username);

    // Call the client class
    ChatClient client;

    // Call the console reader
    ConsoleReader reader;

    // Connect the console reader to the POST method
    QObject::connect(&reader, &ConsoleReader::messageTyped, &client, [username, &client](const QString &msg) {
        client.sendMessage(username, msg);
    });

    // Launch de console reader
    reader.start();

    // Launch the event loop
    return app.exec();
}
