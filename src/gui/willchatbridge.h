#pragma once

#include <QObject>

#include <atomic>
#include <memory>

class MessengerWorker;
class WorkerRuntime;


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

signals:
    void requestConnectDefaultServer();
    void requestSendLine(const QString& line);
    void requestDisconnectServer();

private:
    std::unique_ptr<WorkerRuntime> worker_runtime_;
    std::atomic_bool connected_{false};
};
