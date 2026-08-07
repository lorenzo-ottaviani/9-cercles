#include "ChatClient.hpp"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

// Constructor
ChatClient::ChatClient(QObject *parent) : QObject(parent) {
    // Create a timer and connect it to the fetch history method
    pollTimer = new QTimer(this);
    QObject::connect(pollTimer, &QTimer::timeout, this, &ChatClient::fetchHistory);
    pollTimer->start(5000);
}


// GET methods
void ChatClient::fetchHistory() {
    // Create a Qt request that holds the metadata of the call (URL, Headers, Authentication).
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:18080/chat/history"));

    // Send a GET request, don't wait for the server response.
    QNetworkReply *reply = this->manager.get(request);

    // Call the Callback method to check the server response.
    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        this->onHistoryReceived(reply);
    });
}

void ChatClient::onHistoryReceived(QNetworkReply *reply) {
        // Check if the HTTP transaction was successful or if a network error occurred.
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            qDebug() << "Reply chat history:" << data;

        } else {
            qDebug() << "Failed to retrieve chat history:" << reply->errorString();
        }

        // Delete the reply object and close the application.
        reply->deleteLater();
}

// POST methods
void ChatClient::sendMessage(const QString &user, const QString &message) {
    // Create a Qt request that holds the metadata of the call (URL, Headers, Authentication).
    QNetworkRequest request;
    request.setUrl(QUrl("http://localhost:18080/chat/send"));

    // Prepare a JSON object and convert it into a ByteArray format
    QJsonObject json_data{{"user", user}, {"message", message}};
    QByteArray formatted_data = QJsonDocument(json_data).toJson();

    // Set the request header to indicate that the content type is JSON
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Send a POST request, don't wait for the server response.
    QNetworkReply *reply = this->manager.post(request, formatted_data);

    // Call the Callback method to check the server response.
    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        this->onMessageSent(reply);
    });
 }

void ChatClient::onMessageSent(QNetworkReply *reply) {
    // Check if the HTTP transaction was successful or if a network error occurred.
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Server response:" << data;

    } else {
        qDebug() << "Failed to send message:" << reply->errorString();
    }

    // Delete the reply object and close the application.
    reply->deleteLater();
}
