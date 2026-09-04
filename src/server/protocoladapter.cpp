#include "protocoladapter.h"

#include "inbound_client_message_handler.h"

#include "values/timestamp.h"

#include <iostream>


namespace will {


ProtocolAdapter::ProtocolAdapter(domain::MessengerPersistence persistence, SessionRegistry& registry)
    : persistence_(persistence)
    , registry_(registry)
    , participant_notifier_(registry, persistence.world.heaven())
    , authenticate_device_(persistence.world)
    , send_letter_(persistence.letters, participant_notifier_)
    , fetch_letter_history_(persistence.letters, persistence.world.heaven())
{}


void ProtocolAdapter::on_client_event(const SessionId session_id, const v1::ClientEvent& event)
{
    InboundClientMessageHandler handler{*this, session_id};
    handler.on(event);
}


bool ProtocolAdapter::is_authenticated(const SessionId session_id) const
{
    return registry_.is_authenticated(session_id);
}


void ProtocolAdapter::send_event(const SessionId session_id, const v1::ServerEvent& event)
{
    registry_.enqueue_event(session_id, event);
}


void ProtocolAdapter::close_with_protocol_error(const SessionId session_id, const std::string_view message)
{
    if (const std::string_view peer_address = registry_.peer_address(session_id); !peer_address.empty())
        std::cerr << "Session " << peer_address << ": " << message << '\n';
    else
        std::cerr << "Session " << session_id.value << ": " << message << '\n';

    close_session(session_id);
}


void ProtocolAdapter::close_session(const SessionId session_id)
{
    registry_.close_session(session_id);
}


void ProtocolAdapter::handle_bind_token(const SessionId session_id, const v1::BindToken& token)
{
    const domain::AuthenticateDeviceInput input{token.token()};
    const auto outcome = authenticate_device_.execute(input);

    if (std::holds_alternative<domain::AuthError>(outcome)) {
        send_auth_required(session_id);
        return;
    }

    const auto& success = std::get<domain::AuthenticateDeviceSuccess>(outcome);
    if (const auto displaced = registry_.bind_soul(session_id, success.man.soul_id()))
        close_session(*displaced);

    v1::ServerEvent event;
    event.mutable_auth_ok();
    send_event(session_id, event);
}


void ProtocolAdapter::send_auth_required(const SessionId session_id)
{
    v1::ServerEvent event;
    event.mutable_auth_required();
    send_event(session_id, event);
}


void ProtocolAdapter::handle_user_chat(const SessionId session_id, const v1::ChatMessage& chat)
{
    const domain::SendLetterInput input{*registry_.soul_id(session_id), persistence_.world.abode().id, chat.body(),
                                        domain::Timestamp{}};

    (void)send_letter_.execute(input);

    v1::ServerEvent event;
    event.mutable_receipt_ack();
    send_event(session_id, event);
}


void ProtocolAdapter::handle_history_request(const SessionId session_id, const v1::HistoryRequest& request)
{
    const domain::FetchLetterHistoryInput input{*registry_.soul_id(session_id), persistence_.world.abode().id,
                                                request.limit()};

    const auto outcome = fetch_letter_history_.execute(input);
    if (const auto* error = std::get_if<domain::DomainError>(&outcome)) {
        (void)error;
        close_with_protocol_error(session_id, "Protocol error: invalid HistoryRequest");
        return;
    }

    const auto& history = std::get<domain::FetchLetterHistoryResult>(outcome);
    for (const domain::FetchLetterHistoryItem& item : history.items) {
        v1::ServerEvent event;
        auto* history_item = event.mutable_history_item();
        history_item->set_message_id(item.letter.id().value());
        history_item->set_is_mine(item.is_mine);
        history_item->set_name(item.author_name);
        history_item->set_body(item.letter.body());
        send_event(session_id, event);
    }

    v1::ServerEvent end_event;
    end_event.mutable_history_end();
    send_event(session_id, end_event);
}


} // namespace will
