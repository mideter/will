#include "willchatbridge.h"

#include "messengerclient.h"
#include "defaultwillserver.h"

#include <QMetaObject>

#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <utility>


WillChatBridge::WillChatBridge(QObject* parent)
    : QObject(parent)
{
}


WillChatBridge::~WillChatBridge()
{
    disconnectServer();
}


bool WillChatBridge::isConnected() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<bool>(client_);
}


void WillChatBridge::connectDefaultServer()
{
    disconnectServer();

    std::unique_ptr<MessengerClient> next;
    try {
        next = std::make_unique<MessengerClient>();
        next->connect(defaultWillServerAddress());
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromLocal8Bit(e.what()));
        emit connectionChanged(false);
        return;
    }

    MessengerClient* raw = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        client_ = std::move(next);
        raw = client_.get();
    }

    recv_thread_ = std::jthread([this, raw] { recvLoop(raw); });

    emit connectionChanged(true);
}


void WillChatBridge::sendLine(const QString& line)
{
    std::string bytes = line.toStdString();
    const std::string_view chunk(bytes);

    std::unique_ptr<MessengerClient> dead;
    std::jthread join_me{};
    QString err;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!client_)
            return;

        try {
            client_->send(chunk);
            return;
        } catch (const std::exception& e) {
            err = QString::fromLocal8Bit(e.what());
            try {
                client_->shutdown();
            } catch (...) {
            }
            join_me = std::move(recv_thread_);
            recv_thread_ = std::jthread{};
            dead = std::move(client_);
        }
    }

    if (join_me.joinable())
        join_me.join();

    if (dead) {
        emit errorOccurred(err);
        emit connectionChanged(false);
    }
}


void WillChatBridge::disconnectServer()
{
    std::unique_ptr<MessengerClient> dead;
    std::jthread join_me{};

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!client_)
            return;

        try {
            client_->shutdown();
        } catch (...) {
        }

        join_me = std::move(recv_thread_);
        recv_thread_ = std::jthread{};
        dead = std::move(client_);
    }

    if (join_me.joinable())
        join_me.join();

    emit connectionChanged(false);
}


void WillChatBridge::recvLoop(MessengerClient* c)
{
    try {
        while (true) {
            const std::optional<std::string> incoming = c->receiveMessage();
            
            if (!incoming.has_value())
                break;

            const QString q = QString::fromStdString(*incoming);

            QMetaObject::invokeMethod(
                this,
                "deliverPeer",
                Qt::QueuedConnection,
                Q_ARG(QString, q));
        }
    } catch (const std::exception& e) {
        const QString msg = QString::fromLocal8Bit(e.what());
        QMetaObject::invokeMethod(
            this,
            "deliverError",
            Qt::QueuedConnection,
            Q_ARG(QString, msg));
    }

    QMetaObject::invokeMethod(this, "onRecvThreadDone", Qt::QueuedConnection);
}


void WillChatBridge::deliverPeer(const QString& text)
{
    emit peerMessageReceived(text);
}


void WillChatBridge::deliverError(const QString& message)
{
    emit errorOccurred(message);
}


void WillChatBridge::onRecvThreadDone()
{
    std::unique_ptr<MessengerClient> dead;
    std::jthread join_me{};

    {
        std::lock_guard<std::mutex> lock(mutex_);
        join_me = std::move(recv_thread_);
        recv_thread_ = std::jthread{};
        dead = std::move(client_);
    }

    if (join_me.joinable())
        join_me.join();

    if (dead)
        emit connectionChanged(false);
}
