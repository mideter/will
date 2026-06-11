#include "chat_ui_visitor.h"

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


} // namespace


ChatUiVisitor::ChatUiVisitor(const Context context, const WillClient* client)
    : context_(context)
    , client_(client)
{}


void ChatUiVisitor::on(const ServerMessage& message)
{
    if (dynamic_cast<const LoginResponseMessage*>(&message) != nullptr
        || dynamic_cast<const AuthRequiredMessage*>(&message) != nullptr) {
        if (context_ == Context::LoadingHistory)
            throw_unexpected_history();
        return;
    }

    if (dynamic_cast<const ServerReceiptAckMessage*>(&message) != nullptr) {
        if (context_ == Context::LoadingHistory)
            throw_unexpected_history();

        if (context_ == Context::Receiving && client_ != nullptr && !client_->config().quiet_receipts)
            std::cerr << "[server] ваше сообщение принято" << std::endl;
        return;
    }

    if (const auto* item = dynamic_cast<const HistoryItemMessage*>(&message)) {
        if (context_ == Context::LoadingHistory || context_ == Context::Receiving)
            print_history_item(*item);
        return;
    }

    if (dynamic_cast<const HistoryEndMessage*>(&message) != nullptr) {
        if (context_ == Context::LoadingHistory)
            history_finished_ = true;
        return;
    }

    if (const auto* chat = dynamic_cast<const UserChatMessage*>(&message)) {
        if (context_ == Context::LoadingHistory)
            throw_unexpected_history();

        if (context_ == Context::Receiving) {
            std::cout << chat->body() << std::endl;
            std::cout.flush();
        }
        return;
    }

    throw std::runtime_error("Will protocol: unhandled server message type");
}


void ChatUiVisitor::throw_unexpected_history() const
{
    throw std::runtime_error("Unexpected message while loading history");
}


} // namespace will
