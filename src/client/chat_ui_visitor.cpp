#include "chat_ui_visitor.h"

#include "willclient.h"

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


void ChatUiVisitor::on(const LoginResponseMessage&)
{
    if (context_ == Context::LoadingHistory)
        throw_unexpected_history();
}


void ChatUiVisitor::on(const AuthRequiredMessage&)
{
    if (context_ == Context::LoadingHistory)
        throw_unexpected_history();
}


void ChatUiVisitor::on(const ServerReceiptAckMessage&)
{
    if (context_ == Context::LoadingHistory)
        throw_unexpected_history();

    if (context_ == Context::Receiving && client_ != nullptr && !client_->config().quiet_receipts)
        std::cerr << "[server] ваше сообщение принято" << std::endl;
}


void ChatUiVisitor::on(const HistoryItemMessage& message)
{
    if (context_ == Context::LoadingHistory || context_ == Context::Receiving)
        print_history_item(message);
}


void ChatUiVisitor::on(const HistoryEndMessage&)
{
    if (context_ == Context::LoadingHistory)
        history_finished_ = true;
}


void ChatUiVisitor::on(const UserChatMessage& message)
{
    if (context_ == Context::LoadingHistory)
        throw_unexpected_history();

    if (context_ == Context::Receiving) {
        std::cout << message.body() << std::endl;
        std::cout.flush();
    }
}


void ChatUiVisitor::throw_unexpected_history() const
{
    throw std::runtime_error("Unexpected message while loading history");
}


} // namespace will
