#include "chatsession.h"

#include "inbound_server_message_handler.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

#include "wiremessage_codec.h"


namespace will {


namespace {


bool is_post_auth_server_message(const ServerMessage& message) noexcept
{
    switch (message.type()) {
    case WireMessage::Type::ServerReceiptAck:
    case WireMessage::Type::AuthRequired:
    case WireMessage::Type::UserChat:
    case WireMessage::Type::HistoryItem:
    case WireMessage::Type::HistoryEnd:
    case WireMessage::Type::Ping:
        return true;
    default:
        return false;
    }
}


} // namespace


ChatSession::ChatSession(WillClient& client)
    : client_(client)
{}


void ChatSession::run()
{
    loadHistory();

    std::atomic<bool> disconnected{false};

    client_.set_closed_handler([&disconnected] { disconnected.store(true); });
    client_.set_inbound_handler([this, &disconnected](std::vector<char> payload) {
        if (disconnected.load())
            return;

        try {
            ReceivingMessageHandler handler{client_};
            on_server_payload(payload, handler);
        }
        catch (const std::exception& e) {
            std::cerr << std::endl << "Receive error: " << e.what() << std::endl;
            disconnected.store(true);
        }
    });

    std::cout << "Connected to Will chat. Type messages and press Enter.\n";
    std::cout << "Press Ctrl+D to exit.\n";

    std::string line;
    while (!disconnected.load() && std::getline(std::cin, line))
        client_.send(line);

    if (disconnected.load())
        std::cout << std::endl << "Disconnected from chat." << std::endl;

    client_.shutdown();
}


void ChatSession::loadHistory() const
{
    if (client_.config().history_limit == 0)
        return;

    std::mutex mutex;
    std::condition_variable cv;
    bool finished = false;
    bool disconnected = false;
    std::string error_message;

    client_.set_closed_handler([&] {
        std::lock_guard lock(mutex);
        if (!finished) {
            disconnected = true;
            cv.notify_one();
        }
    });

    client_.set_inbound_handler([&](std::vector<char> payload) {
        try {
            LoadingHistoryMessageHandler handler;
            on_server_payload(payload, handler);

            std::lock_guard lock(mutex);
            if (handler.history_finished())
                finished = true;
            cv.notify_one();
        }
        catch (const std::exception& e) {
            std::lock_guard lock(mutex);
            error_message = e.what();
            disconnected = true;
            cv.notify_one();
        }
    });

    client_.requestHistory(client_.config().history_limit);

    std::unique_lock lock(mutex);
    cv.wait(lock, [&] { return finished || disconnected; });

    client_.set_closed_handler(nullptr);
    client_.set_inbound_handler(nullptr);

    if (!finished)
        throw std::runtime_error(disconnected && !error_message.empty() ? error_message
                                                                        : "Disconnected while loading history");
}


template<typename Handler>
void ChatSession::on_server_payload(const std::vector<char>& payload, Handler& handler) const
{
    const auto message = WireMessageCodec::decode_server(payload);
    if (!message || !is_post_auth_server_message(*message)) {
        throw std::runtime_error("Will protocol: invalid server frame");
    }

    handler.on(*message);
}


template void ChatSession::on_server_payload<LoadingHistoryMessageHandler>(const std::vector<char>&,
                                                                           LoadingHistoryMessageHandler&) const;
template void ChatSession::on_server_payload<ReceivingMessageHandler>(const std::vector<char>&,
                                                                      ReceivingMessageHandler&) const;


} // namespace will
