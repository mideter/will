#include "protocoladapter.h"

#include "inbound_client_message_handler.h"
#include "sessionregistry.h"

#include "support/timestamp.h"

#include <iostream>


namespace will {


ProtocolAdapter::ProtocolAdapter(domain::MessengerPersistence persistence, SessionRegistry& registry,
                                 ConnectionAccountStore& account_store)
    : persistence_(persistence)
    , registry_(registry)
    , account_store_(account_store)
    , participant_notifier_(registry)
    , authenticate_device_(persistence.users)
    , send_chat_message_(persistence.messages, participant_notifier_)
    , fetch_chat_history_(persistence.messages)
{}


void ProtocolAdapter::on_client_event(const std::uint64_t session_id, const v1::ClientEvent& event)
{
    InboundClientMessageHandler handler{*this, session_id};
    handler.on(event);
}


void ProtocolAdapter::send_event(const std::uint64_t session_id, const v1::ServerEvent& event)
{
    registry_.enqueue_event(session_id, event);
}


void ProtocolAdapter::close_with_protocol_error(const std::uint64_t session_id, const std::string_view message)
{
    if (const std::string_view peer_address = registry_.peer_address(session_id); !peer_address.empty())
        std::cerr << "Session " << peer_address << ": " << message << '\n';
    else
        std::cerr << "Session " << session_id << ": " << message << '\n';

    close_session(session_id);
}


void ProtocolAdapter::close_session(const std::uint64_t session_id)
{
    registry_.close_session(session_id);
}


void ProtocolAdapter::handle_bind_token(const std::uint64_t session_id, const v1::BindToken& token)
{
    const domain::AuthenticateDeviceInput input{token.token(), domain::Timestamp::now()};
    const auto outcome = authenticate_device_.execute(input);

    if (std::holds_alternative<domain::AuthError>(outcome)) {
        send_auth_required(session_id);
        return;
    }

    const auto& success = std::get<domain::AuthenticateDeviceSuccess>(outcome);
    if (const auto displaced = account_store_.set(session_id, success.account))
        close_session(*displaced);

    v1::ServerEvent event;
    event.mutable_auth_ok();
    send_event(session_id, event);
}


void ProtocolAdapter::send_auth_required(const std::uint64_t session_id)
{
    v1::ServerEvent event;
    event.mutable_auth_required();
    send_event(session_id, event);
}


void ProtocolAdapter::handle_user_chat(const std::uint64_t session_id, const v1::ChatMessage& chat)
{
    const domain::SendChatMessageInput input{
        *account_store_.get(session_id),
        domain::ParticipantId{session_id},
        domain::ChatId::global(),
        chat.body(),
        domain::Timestamp::now(),
    };

    (void)send_chat_message_.execute(input);

    v1::ServerEvent event;
    event.mutable_receipt_ack();
    send_event(session_id, event);
}


void ProtocolAdapter::handle_history_request(const std::uint64_t session_id, const v1::HistoryRequest& request)
{
    const domain::FetchChatHistoryInput input{*account_store_.get(session_id), domain::ChatId::global(),
                                              request.limit()};

    const auto outcome = fetch_chat_history_.execute(input);
    if (const auto* error = std::get_if<domain::DomainError>(&outcome)) {
        (void)error;
        close_with_protocol_error(session_id, "Protocol error: invalid HistoryRequest");
        return;
    }

    const auto& history = std::get<domain::FetchChatHistoryResult>(outcome);
    for (const domain::FetchChatHistoryItem& item : history.items) {
        v1::ServerEvent event;
        auto* history_item = event.mutable_history_item();
        history_item->set_message_id(item.message.id);
        history_item->set_is_mine(item.is_mine);
        history_item->set_name(item.message.author_name);
        history_item->set_body(item.message.body);
        send_event(session_id, event);
    }

    v1::ServerEvent end_event;
    end_event.mutable_history_end();
    send_event(session_id, end_event);
}


} // namespace will
