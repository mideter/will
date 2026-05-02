#pragma once

#include <QObject>

#include <memory>
#include <mutex>
#include <thread>


class MessengerClient;


class WillChatBridge final : public QObject {
    Q_OBJECT

public:
    explicit WillChatBridge(QObject* parent = nullptr);
    ~WillChatBridge() override;

    bool isConnected() const;

public slots:
    void connectDefaultServer();
    void sendLine(const QString& line);
    void disconnectServer();

signals:
    void peerMessageReceived(const QString& text);
    void errorOccurred(const QString& message);
    void connectionChanged(bool connected);

private slots:
    void deliverPeer(const QString& text);
    void deliverError(const QString& message);
    void onRecvThreadDone();

private:
    void recvLoop(MessengerClient* client);

    mutable std::mutex mutex_;
    std::unique_ptr<MessengerClient> client_;
    std::jthread recv_thread_;
};
