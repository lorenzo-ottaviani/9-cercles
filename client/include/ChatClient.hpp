#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QTimer>

class ChatClient : public QObject {
    Q_OBJECT

private:
    // Create a network manager to handles the underlying network stack.
    // It works asynchronously, meaning it doesn't freeze the program while waiting for the server.
    QNetworkAccessManager manager;

    // Qt timer used to refresh the history
    QTimer* pollTimer;

    // Callback for the GET method to retrieve the chat history
    void onHistoryReceived(QNetworkReply *reply);

    // Callback for the POST method to send a message
    void onMessageSent(QNetworkReply *reply);

public:
    // Constructor
    ChatClient(QObject *parent = nullptr);

    // GET method to retrieve the chat history from the server
    void fetchHistory();

    // POST method to send a message to the server
    void sendMessage(const QString& user, const QString& message);

};
