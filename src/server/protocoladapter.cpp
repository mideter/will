#include "protocoladapter.h"

#include "tcpconnectionregistry.h"

#include <chrono>
#include <iostream>
#include <string_view>
#include <variant>

#include "willprotocol.h"
#include "wiremessage.h"


namespace will {


namespace {


std::uint8_t login_error_code_for(const domain::AuthResult result)
{
    switch (result) {
    case domain::AuthResult::ExpiredToken:
        return WireMessage::LoginErrorExpiredToken;
    case domain::AuthResult::InvalidCredentials:
    default:
        return WireMessage::LoginErrorInvalidCredentials;
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
    if (WireMessage::is_login_request(payload)) {
        handle_login(connection_id, payload);
        return;
    }

    if (WireMessage::is_bind_token(payload)) {
        handle_bind_token(connection_id, payload);
        return;
    }

    if (WireMessage::is_user_chat(payload) || WireMessage::is_history_request(payload)) {
        if (!account_store_.has(connection_id)) {
            send_auth_required(connection_id);
            return;
        }

        if (WireMessage::is_user_chat(payload)) {
            handle_user_chat(connection_id, payload);
            return;
        }

        handle_history_request(connection_id, payload);
        return;
    }

    if (!WireMessage::is_valid_client_to_server_payload(payload)) {
        close_with_protocol_error(connection_id, "Protocol error: invalid client frame");
        return;
    }

    close_with_protocol_error(connection_id, "Protocol error: invalid client frame");
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


void ProtocolAdapter::handle_login(const std::uint64_t connection_id, const std::vector<char>& payload)
{
    const auto request = WireMessage::parse_login_request(payload);
    if (!request) {
        close_with_protocol_error(connection_id, "Protocol error: invalid LoginRequest");
        return;
    }

    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::AuthenticateUserInput input{request->login, request->password, now_ms};
    const auto outcome = authenticate_user_.execute(input);

    if (const auto* failure = std::get_if<domain::AuthResult>(&outcome)) {
        send_payload(connection_id,
                     WireMessage::encode_login_response_failure(login_error_code_for(*failure)));
        return;
    }

    const auto& success = std::get<domain::AuthenticateUserSuccess>(outcome);
    send_payload(connection_id,
                 WireMessage::encode_login_response_success(success.account.session_token.value));
}


void ProtocolAdapter::handle_bind_token(const std::uint64_t connection_id, const std::vector<char>& payload)
{
    const auto token = WireMessage::parse_bind_token(payload);
    if (!token) {
        close_with_protocol_error(connection_id, "Protocol error: invalid BindToken");
        return;
    }

    const auto account = persistence_.sessions.resolve_token(domain::AuthToken{*token});
    if (!account) {
        send_payload(connection_id,
                     WireMessage::encode_login_response_failure(WireMessage::LoginErrorExpiredToken));
        return;
    }

    account_store_.set(connection_id, *account);
}


void ProtocolAdapter::send_auth_required(const std::uint64_t connection_id)
{
    send_payload(connection_id, WireMessage::encode_auth_required());
}


void ProtocolAdapter::handle_user_chat(const std::uint64_t connection_id, const std::vector<char>& payload)
{
    const std::string body(payload.begin() + 1, payload.end());
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::SendChatMessageInput input{
        *account_store_.get(connection_id),
        domain::ParticipantId{connection_id},
        domain::ChatId::global(),
        body,
        now_ms,
    };

    (void)send_chat_message_.execute(input);
    send_payload(connection_id, WireMessage::encode_server_receipt_ack());
}


void ProtocolAdapter::handle_history_request(const std::uint64_t connection_id,
                                             const std::vector<char>& payload)
{
    const auto limit = WireMessage::parse_history_request_limit(payload);
    if (!limit) {
        close_with_protocol_error(connection_id, "Protocol error: invalid HistoryRequest");
        return;
    }

    const domain::FetchChatHistoryInput input{*account_store_.get(connection_id), domain::ChatId::global(),
                                              *limit};

    const auto outcome = fetch_chat_history_.execute(input);
    if (const auto* error = std::get_if<domain::DomainError>(&outcome)) {
        (void)error;
        close_with_protocol_error(connection_id, "Protocol error: invalid HistoryRequest");
        return;
    }

    const auto& history = std::get<domain::FetchChatHistoryResult>(outcome);
    for (const domain::FetchChatHistoryItem& item : history.items) {
        send_payload(connection_id, WireMessage::encode_history_item(item.message.id, item.is_mine,
                                                                     item.message.body));
    }

    send_payload(connection_id, WireMessage::encode_history_end());
}


std::vector<char> ProtocolAdapter::encode_user_chat(const std::string_view utf8_body)
{
    return WireMessage::encode_user_chat(utf8_body);
}


std::string ProtocolAdapter::format_payload_for_log(const std::vector<char>& payload)
{
    return WireMessage::format_payload_for_log(payload);
}


} // namespace will
