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


namespace {


std::uint8_t login_error_code_for(const domain::AuthResult result)
{
    switch (result) {
    case domain::AuthResult::ExpiredToken:
        return static_cast<std::uint8_t>(LoginResponseMessage::Error::ExpiredToken);
    case domain::AuthResult::InvalidCredentials:
    default:
        return static_cast<std::uint8_t>(LoginResponseMessage::Error::InvalidCredentials);
    }
}


} // namespace


ProtocolAdapter::ProtocolAdapter(domain::MessengerPersistence persistence, TcpConnectionRegistry& registry,
                                 ConnectionAccountStore& account_store)
    : persistence_(persistence)
    , registry_(registry)
    , account_store_(account_store)
    , participant_notifier_(registry)
    , authenticate_user_(persistence.users, persistence.sessions)
    , send_chat_message_(persistence.messages, participant_notifier_)
    , fetch_chat_history_(persistence.messages)
{}


void ProtocolAdapter::on_client_frame(const std::uint64_t connection_id, const std::vector<char>& payload)
{
    const auto message = WireMessageCodec::decode_client(payload);
    if (!message) {
        close_with_protocol_error(connection_id, "Protocol error: invalid client frame");
        return;
    }

    InboundClientMessageHandler handler{*this, connection_id};
    handler.on(*message);
}


void ProtocolAdapter::send_payload(const std::uint64_t connection_id, const std::vector<char>& app_payload)
{
    registry_.enqueue_wire_frame(connection_id, TcpFrame::encode(app_payload));
}


void ProtocolAdapter::close_with_protocol_error(const std::uint64_t connection_id,
                                                const std::string_view message)
{
    if (const std::string_view label = registry_.peer_label(connection_id); !label.empty())
        std::cerr << "Connection " << label << ": " << message << '\n';
    else
        std::cerr << "Connection " << connection_id << ": " << message << '\n';
    
    registry_.close_connection(connection_id);
}


void ProtocolAdapter::handle_login(const std::uint64_t connection_id, const LoginRequestMessage& request)
{
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::AuthenticateUserInput input{request.login(), request.password(), now_ms};
    const auto outcome = authenticate_user_.execute(input);

    if (const auto* failure = std::get_if<domain::AuthResult>(&outcome)) {
        send_payload(connection_id,
                     WireMessageCodec::encode(LoginResponseMessage{false, "", login_error_code_for(*failure)}));
        return;
    }

    const auto& success = std::get<domain::AuthenticateUserSuccess>(outcome);
    send_payload(connection_id, WireMessageCodec::encode(LoginResponseMessage{true, success.account.session_token.value, 0}));
}


void ProtocolAdapter::handle_bind_token(const std::uint64_t connection_id, const BindTokenMessage& token)
{
    const auto account = persistence_.sessions.resolve_token(domain::AuthToken{token.token()});
    if (!account) {
        send_payload(connection_id,
                     WireMessageCodec::encode(LoginResponseMessage{
                         false, "", static_cast<std::uint8_t>(LoginResponseMessage::Error::ExpiredToken)}));
        return;
    }

    account_store_.set(connection_id, *account);
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
