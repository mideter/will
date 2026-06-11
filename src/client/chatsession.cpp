#include "chatsession.h"

#include "chat_ui_visitor.h"

#include <atomic>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>


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
        const std::optional<std::unique_ptr<ServerMessage>> incoming = client_.receiveMessage();

        if (!incoming.has_value())
            throw std::runtime_error("Disconnected while loading history");

        ChatUiVisitor visitor{ChatUiVisitor::Context::LoadingHistory};
        (*incoming)->accept(visitor);

        if (visitor.history_finished())
            break;
    }
}


void ChatSession::receiveLoop() const
{
    try {
        while (true) {
            const std::optional<std::unique_ptr<ServerMessage>> incoming = client_.receiveMessage();

            if (!incoming.has_value()) {
                std::cout << std::endl << "Disconnected from chat." << std::endl;
                break;
            }

            ChatUiVisitor visitor{ChatUiVisitor::Context::Receiving, &client_};
            (*incoming)->accept(visitor);
        }
    }
    catch (const std::exception& e) {
        std::cerr << std::endl << "Receive error: " << e.what() << std::endl;
    }
}


} // namespace will
