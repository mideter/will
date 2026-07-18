#include "inbound_server_message_handler.h"

#include "consoleui.h"
#include "willclient.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"

#include <stdexcept>


namespace will {


namespace {


void print_history_item(ConsoleUi& ui, const HistoryItemMessage& item)
{
    if (item.is_mine())
        ui.print_mine(item.body(), true);
    else
        ui.print_peer(item.body(), true);
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


LoadingHistoryMessageHandler::LoadingHistoryMessageHandler(ConsoleUi& ui)
    : ui_(ui)
{}


void LoadingHistoryMessageHandler::on(const ServerMessage& message)
{
    if (const auto* item = dynamic_cast<const HistoryItemMessage*>(&message)) {
        print_history_item(ui_, *item);
        return;
    }

    if (dynamic_cast<const HistoryEndMessage*>(&message) != nullptr) {
        history_finished_ = true;
        return;
    }

    throw_unexpected_history_message();
}


ReceivingMessageHandler::ReceivingMessageHandler(const WillClient& client, ConsoleUi& ui)
    : client_(client)
    , ui_(ui)
{}


void ReceivingMessageHandler::on(const ServerMessage& message)
{
    if (dynamic_cast<const AuthOkMessage*>(&message) != nullptr
        || dynamic_cast<const AuthRequiredMessage*>(&message) != nullptr)
        return;

    if (dynamic_cast<const ServerReceiptAckMessage*>(&message) != nullptr) {
        if (!client_.config().quiet_receipts)
            ui_.print_receipt();
        return;
    }

    if (dynamic_cast<const PingMessage*>(&message) != nullptr)
        return;

    if (const auto* item = dynamic_cast<const HistoryItemMessage*>(&message)) {
        print_history_item(ui_, *item);
        return;
    }

    if (const auto* chat = dynamic_cast<const UserChatMessage*>(&message)) {
        ui_.print_peer(chat->body());
        return;
    }

    if (dynamic_cast<const HistoryEndMessage*>(&message) != nullptr)
        return;

    throw_unhandled_server_message();
}


} // namespace will
