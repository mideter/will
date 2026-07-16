#include "inbound_server_message_handler.h"

#include "willclient.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"

#include <iostream>
#include <stdexcept>


namespace will {


namespace {


void print_history_item(const HistoryItemMessage& item)
{
    if (item.is_mine())
        std::cout << "[me] " << item.body() << '\n';
    else
        std::cout << "[peer] " << item.body() << '\n';
}


[[noreturn]] void throw_unexpected_history_message()
{
    throw std::runtime_error("Unexpected message while loading history");
}


[[noreturn]] void throw_unhandled_server_message()
{
    throw std::runtime_error("Will protocol: unhandled server message type");
}


} // namespace


void LoadingHistoryMessageHandler::on(const ServerMessage& message)
{
    if (const auto* item = dynamic_cast<const HistoryItemMessage*>(&message)) {
        print_history_item(*item);
        return;
    }

    if (dynamic_cast<const HistoryEndMessage*>(&message) != nullptr) {
        history_finished_ = true;
        return;
    }

    throw_unexpected_history_message();
}


ReceivingMessageHandler::ReceivingMessageHandler(const WillClient& client)
    : client_(client)
{}


void ReceivingMessageHandler::on(const ServerMessage& message)
{
    if (dynamic_cast<const AuthOkMessage*>(&message) != nullptr
        || dynamic_cast<const AuthRequiredMessage*>(&message) != nullptr)
        return;

    if (dynamic_cast<const ServerReceiptAckMessage*>(&message) != nullptr) {
        if (!client_.config().quiet_receipts)
            std::cerr << "[server] ваше сообщение принято" << std::endl;
        return;
    }

    if (const auto* item = dynamic_cast<const HistoryItemMessage*>(&message)) {
        print_history_item(*item);
        return;
    }

    if (const auto* chat = dynamic_cast<const UserChatMessage*>(&message)) {
        std::cout << chat->body() << std::endl;
        std::cout.flush();
        return;
    }

    if (dynamic_cast<const HistoryEndMessage*>(&message) != nullptr)
        return;

    throw_unhandled_server_message();
}


} // namespace will
