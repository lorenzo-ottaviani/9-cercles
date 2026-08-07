#pragma once

#include <QThread>

class ConsoleReader : public QThread {
    Q_OBJECT

signals:
    // Signal emitted by the thread to trigger the POST method
    void messageTyped(const QString &message);

protected:
    // Called by the ConsolReader thread
    void run() override;
};
