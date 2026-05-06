#include "messengerworker.h"

#include "defaultwillserver.h"
#include "messengerclient.h"

#include <QMetaObject>

#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <utility>


MessengerWorker::MessengerWorker(QObject* parent)
    : QObject(parent)
{}


MessengerWorker::~MessengerWorker()
{
    disconnectServer();
}


void MessengerWorker::connectDefaultServer()
{
    disconnectServer();

    std::unique_ptr<will::MessengerClient> next;
    try {
        next = std::make_unique<will::MessengerClient>();
        next->connect(will::defaultWillServerAddress());
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromLocal8Bit(e.what()));
        emit connectionChanged(false);
        return;
    }

    will::MessengerClient* raw = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_requested_.store(false);
        client_ = std::move(next);
        raw = client_.get();
    }

    connected_.store(true);
    recv_thread_ = std::jthread([this, raw] { recvLoop(raw); });
    emit connectionChanged(true);
}


void MessengerWorker::sendLine(const QString& line)
{
    std::string bytes = line.toStdString();
    const std::string_view chunk(bytes);

    std::jthread join_me{};
    std::unique_ptr<will::MessengerClient> dead;
    QString err;
    bool was_connected = false;

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
            stop_requested_.store(true);
            was_connected = connected_.exchange(false);
            join_me = std::move(recv_thread_);
            recv_thread_ = std::jthread{};
            dead = std::move(client_);
        }
    }

    if (join_me.joinable())
        join_me.join();

    if (dead) {
        emit errorOccurred(err);
        if (was_connected)
            emit connectionChanged(false);
    }
}


void MessengerWorker::disconnectServer()
{
    std::jthread join_me{};
    std::unique_ptr<will::MessengerClient> dead;

    const bool was_connected = connected_.exchange(false);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_requested_.store(true);

        if (client_) {
            try {
                client_->shutdown();
            } catch (...) {
            }
        }

        join_me = std::move(recv_thread_);
        recv_thread_ = std::jthread{};
        dead = std::move(client_);
    }

    if (join_me.joinable())
        join_me.join();

    if (was_connected)
        emit connectionChanged(false);
}


void MessengerWorker::recvLoop(will::MessengerClient* c)
{
    try {
        while (!stop_requested_.load()) {
            const std::optional<std::string> incoming = c->receiveMessage();
            if (!incoming.has_value())
                break;

            emit peerMessageReceived(QString::fromStdString(*incoming));
        }
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromLocal8Bit(e.what()));
    }

    QMetaObject::invokeMethod(this, [this] { finalizeDisconnectFromRecv(); }, Qt::QueuedConnection);
}


void MessengerWorker::finalizeDisconnectFromRecv()
{
    std::jthread join_me{};
    std::unique_ptr<will::MessengerClient> dead;
    const bool was_connected = connected_.exchange(false);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_requested_.store(true);
        join_me = std::move(recv_thread_);
        recv_thread_ = std::jthread{};
        dead = std::move(client_);
    }

    if (join_me.joinable())
        join_me.join();

    if (was_connected)
        emit connectionChanged(false);
}
