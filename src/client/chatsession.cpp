#include "chatsession.h"

#include <atomic>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <variant>


namespace will {


ChatSession::ChatSession(WillClient& client)
    : client_(client)
{}


void ChatSession::run()
{
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


void ChatSession::receiveLoop() const
{
    try {
        while (true) {
            const std::optional<InboundMessage> incoming = client_.receiveMessage();

            if (!incoming.has_value()) {
                std::cout << std::endl << "Disconnected from chat." << std::endl;
                break;
            }

            if (std::holds_alternative<ServerReceiptAck>(*incoming)) {
                std::cerr << "[server] ваше сообщение принято\n";
                continue;
            }

            std::cout << std::get<std::string>(*incoming) << std::endl;
            std::cout.flush();
        }
    }
    catch (const std::exception& e) {
        std::cerr << std::endl << "Receive error: " << e.what() << std::endl;
    }
}


} // namespace will
