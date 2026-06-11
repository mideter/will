#include "chatsession.h"

#include "inbound_server_message_handler.h"

#include <atomic>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

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

    std::cout << "Connected to Will chat. Type messages and press Enter.\n";
    std::cout << "Press Ctrl+D to exit.\n";

    std::atomic<bool> receive_finished{false};
    std::jthread receiver([this, &receive_finished]() {
        receiveLoop();
        receive_finished.store(true);
    });

    std::string line;
    while (!receive_finished.load() && std::getline(std::cin, line))
        client_.send(line);

    client_.shutdown();
}


void ChatSession::loadHistory() const
{
    if (!client_.requestHistory(client_.config().history_limit))
        return;

    while (true) {
        const std::optional<std::vector<char>> frame = client_.receiveFrame();

        if (!frame.has_value())
            throw std::runtime_error("Disconnected while loading history");

        LoadingHistoryMessageHandler handler;
        on_server_frame(*frame, handler);

        if (handler.history_finished())
            break;
    }
}


void ChatSession::receiveLoop() const
{
    try {
        while (true) {
            const std::optional<std::vector<char>> frame = client_.receiveFrame();

            if (!frame.has_value()) {
                std::cout << std::endl << "Disconnected from chat." << std::endl;
                break;
            }

            ReceivingMessageHandler handler{client_};
            on_server_frame(*frame, handler);
        }
    }
    catch (const std::exception& e) {
        std::cerr << std::endl << "Receive error: " << e.what() << std::endl;
    }
}


template<typename Handler>
void ChatSession::on_server_frame(const std::vector<char>& payload, Handler& handler) const
{
    const auto message = WireMessageCodec::decode_server(payload);
    if (!message || !is_post_auth_server_message(*message)) {
        throw std::runtime_error("Will protocol: invalid server frame");
    }

    handler.on(*message);
}


template void ChatSession::on_server_frame<LoadingHistoryMessageHandler>(const std::vector<char>&,
                                                                           LoadingHistoryMessageHandler&) const;
template void ChatSession::on_server_frame<ReceivingMessageHandler>(const std::vector<char>&,
                                                                    ReceivingMessageHandler&) const;


} // namespace will
