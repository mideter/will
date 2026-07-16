#include "protocoladapter.h"

#include "inbound_client_message_handler.h"
#include "tcpconnectionregistry.h"

#include <chrono>
#include <iostream>
#include <string_view>

#include "tcpframe.h"
#include "wiremessage_codec.h"
#include "wiremessage_server.h"


namespace will {


ProtocolAdapter::ProtocolAdapter(domain::MessengerPersistence persistence, TcpConnectionRegistry& registry,
                                 ConnectionAccountStore& account_store)
    : persistence_(persistence)
    , registry_(registry)
    , account_store_(account_store)
    , participant_notifier_(registry)
    , authenticate_device_(persistence.users)
    , send_chat_message_(persistence.messages, participant_notifier_)
    , fetch_chat_history_(persistence.messages)
{}


void ProtocolAdapter::on_client_payload(const std::uint64_t connection_id, const std::vector<char>& payload)
{
    const auto message = WireMessageCodec::decode_client(payload);
    if (!message) {
        close_with_protocol_error(connection_id, "Protocol error: invalid client frame");
        return;
    }

    InboundClientMessageHandler handler{*this, connection_id};
    handler.on(*message);
}


void ProtocolAdapter::send_payload(const std::uint64_t connection_id, const std::vector<char>& payload)
{
    registry_.enqueue_wire_frame(connection_id, TcpFrame::encode(payload));
}


void ProtocolAdapter::close_with_protocol_error(const std::uint64_t connection_id, const std::string_view message)
{
    if (const std::string_view peer_address = registry_.peer_address(connection_id); !peer_address.empty())
        std::cerr << "Connection " << peer_address << ": " << message << '\n';
    else
        std::cerr << "Connection " << connection_id << ": " << message << '\n';

    close_connection(connection_id);
}


void ProtocolAdapter::close_connection(const std::uint64_t connection_id)
{
    registry_.close_connection(connection_id);
}


void ProtocolAdapter::handle_bind_token(const std::uint64_t connection_id, const BindTokenMessage& token)
{
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::AuthenticateDeviceInput input{token.token(), now_ms};
    const auto outcome = authenticate_device_.execute(input);

    if (std::holds_alternative<domain::AuthError>(outcome)) {
        send_auth_required(connection_id);
        return;
    }

    const auto& success = std::get<domain::AuthenticateDeviceSuccess>(outcome);
    if (const auto displaced = account_store_.set(connection_id, success.account))
        close_connection(*displaced);
    send_payload(connection_id, WireMessageCodec::encode(AuthOkMessage{}));
}


void ProtocolAdapter::send_auth_required(const std::uint64_t connection_id)
{
    send_payload(connection_id, WireMessageCodec::encode(AuthRequiredMessage{}));
}


void ProtocolAdapter::handle_user_chat(const std::uint64_t connection_id, const UserChatMessage& chat)
{
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::SendChatMessageInput input{
        *account_store_.get(connection_id),
        domain::ParticipantId{connection_id},
        domain::ChatId::global(),
        chat.body(),
        now_ms,
    };

    (void)send_chat_message_.execute(input);
    send_payload(connection_id, WireMessageCodec::encode(ServerReceiptAckMessage{}));
}


void ProtocolAdapter::handle_history_request(const std::uint64_t connection_id,
                                             const HistoryRequestMessage& request)
{
    const domain::FetchChatHistoryInput input{*account_store_.get(connection_id), domain::ChatId::global(),
                                              request.limit()};

    const auto outcome = fetch_chat_history_.execute(input);
    if (const auto* error = std::get_if<domain::DomainError>(&outcome)) {
        (void)error;
        close_with_protocol_error(connection_id, "Protocol error: invalid HistoryRequest");
        return;
    }

    const auto& history = std::get<domain::FetchChatHistoryResult>(outcome);
    for (const domain::FetchChatHistoryItem& item : history.items) {
        send_payload(connection_id, WireMessageCodec::encode(HistoryItemMessage{item.message.id, item.is_mine, item.message.body}));
    }

    send_payload(connection_id, WireMessageCodec::encode(HistoryEndMessage{}));
}


} // namespace will
