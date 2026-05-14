#pragma once

#include <QObject>
#include <QString>

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>


namespace will {
class MessengerClient;
}


class MessengerWorker final : public QObject {
    Q_OBJECT

public:
    explicit MessengerWorker(QObject* parent = nullptr);
    ~MessengerWorker() override;

public slots:
    void connectDefaultServer();
    void sendLine(const QString& line);
    void disconnectServer();

signals:
    void peerMessageReceived(const QString& text);
    void serverReceiptConfirmed();
    void errorOccurred(const QString& message);
    void connectionChanged(bool connected);

private:
    void recvLoop(will::MessengerClient* client);
    void finalizeDisconnectFromRecv();

    std::mutex mutex_;
    std::unique_ptr<will::MessengerClient> client_;
    std::jthread recv_thread_;
    std::atomic_bool stop_requested_{false};
    std::atomic_bool connected_{false};
};
