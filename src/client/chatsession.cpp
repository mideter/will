#include "chatsession.h"

#include <atomic>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>


namespace will {


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
        const std::optional<WireMessageEntity> incoming = client_.receiveMessage();
        if (!incoming.has_value())
            throw std::runtime_error("Disconnected while loading history");

        if (std::holds_alternative<HistoryEnd>(*incoming))
            break;

        if (const auto* item = std::get_if<HistoryItemPayload>(&*incoming)) {
            printHistoryItem(*item);
            continue;
        }

        throw std::runtime_error("Unexpected message while loading history");
    }
}


void ChatSession::printHistoryItem(const HistoryItemPayload& item)
{
    if (item.is_mine)
        std::cout << "[me] " << item.body << '\n';
    else
        std::cout << "[peer] " << item.body << '\n';
}


void ChatSession::receiveLoop() const
{
    try {
        while (true) {
            const std::optional<WireMessageEntity> incoming = client_.receiveMessage();

            if (!incoming.has_value()) {
                std::cout << std::endl << "Disconnected from chat." << std::endl;
                break;
            }

            if (std::holds_alternative<ServerReceiptAck>(*incoming)) {
                if (!client_.config().quiet_receipts)
                    std::cerr << "[server] ваше сообщение принято" << std::endl;
                continue;
            }

            if (const auto* item = std::get_if<HistoryItemPayload>(&*incoming)) {
                printHistoryItem(*item);
                continue;
            }

            if (std::holds_alternative<HistoryEnd>(*incoming))
                continue;

            if (const auto* chat = std::get_if<UserChat>(&*incoming)) {
                std::cout << chat->body << std::endl;
                std::cout.flush();
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << std::endl << "Receive error: " << e.what() << std::endl;
    }
}


} // namespace will
