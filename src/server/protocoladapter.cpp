#include "protocoladapter.h"

#include "tcpconnection.h"

#include <chrono>
#include <string_view>
#include <variant>

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


ProtocolAdapter::ProtocolAdapter(domain::MessengerPersistence persistence, TcpConnectionRegistry& registry)
    : persistence_(persistence)
    , participant_notifier_(registry)
    , authenticate_user_(persistence.users, persistence.sessions)
    , send_chat_message_(persistence.messages, participant_notifier_)
    , fetch_chat_history_(persistence.messages)
{}


void ProtocolAdapter::on_client_frame(TcpConnection& connection, const std::vector<char>& payload)
{
    if (WireMessage::is_login_request(payload)) {
        handle_login(connection, payload);
        return;
    }

    if (WireMessage::is_bind_token(payload)) {
        handle_bind_token(connection, payload);
        return;
    }

    if (WireMessage::is_user_chat(payload) || WireMessage::is_history_request(payload)) {
        if (!connection.has_account()) {
            send_auth_required(connection);
            return;
        }

        if (WireMessage::is_user_chat(payload)) {
            handle_user_chat(connection, payload);
            return;
        }

        handle_history_request(connection, payload);
        return;
    }

    if (!WireMessage::is_valid_client_to_server_payload(payload)) {
        connection.fail_protocol("Protocol error: invalid client frame");
        return;
    }

    connection.fail_protocol("Protocol error: invalid client frame");
}


void ProtocolAdapter::handle_login(TcpConnection& connection, const std::vector<char>& payload)
{
    const auto request = WireMessage::parse_login_request(payload);
    if (!request) {
        connection.fail_protocol("Protocol error: invalid LoginRequest");
        return;
    }

    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::AuthenticateUserInput input{request->login, request->password, now_ms};
    const auto outcome = authenticate_user_.execute(input);

    if (const auto* failure = std::get_if<domain::AuthResult>(&outcome)) {
        connection.send_frame(
            WireMessage::encode_login_response_failure(login_error_code_for(*failure)));
        return;
    }

    const auto& success = std::get<domain::AuthenticateUserSuccess>(outcome);
    connection.send_frame(
        WireMessage::encode_login_response_success(success.account.session_token.value));
}


void ProtocolAdapter::handle_bind_token(TcpConnection& connection, const std::vector<char>& payload)
{
    const auto token = WireMessage::parse_bind_token(payload);
    if (!token) {
        connection.fail_protocol("Protocol error: invalid BindToken");
        return;
    }

    const auto account = persistence_.sessions.resolve_token(domain::AuthToken{*token});
    if (!account) {
        connection.send_frame(
            WireMessage::encode_login_response_failure(WireMessage::LoginErrorExpiredToken));
        return;
    }

    connection.set_account(*account);
}


void ProtocolAdapter::send_auth_required(TcpConnection& connection)
{
    connection.send_frame(WireMessage::encode_auth_required());
}


void ProtocolAdapter::handle_user_chat(TcpConnection& sender, const std::vector<char>& payload)
{
    const std::string body(payload.begin() + 1, payload.end());
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::SendChatMessageInput input{
        *sender.account(),
        domain::ParticipantId{sender.id()},
        domain::ChatId::global(),
        body,
        now_ms,
    };

    (void)send_chat_message_.execute(input);
    sender.send_frame(WireMessage::encode_server_receipt_ack());
}


void ProtocolAdapter::handle_history_request(TcpConnection& sender, const std::vector<char>& payload)
{
    const auto limit = WireMessage::parse_history_request_limit(payload);
    if (!limit) {
        sender.fail_protocol("Protocol error: invalid HistoryRequest");
        return;
    }

    const domain::FetchChatHistoryInput input{*sender.account(), domain::ChatId::global(), *limit};

    const auto outcome = fetch_chat_history_.execute(input);
    if (const auto* error = std::get_if<domain::DomainError>(&outcome)) {
        (void)error;
        sender.fail_protocol("Protocol error: invalid HistoryRequest");
        return;
    }

    const auto& history = std::get<domain::FetchChatHistoryResult>(outcome);
    for (const domain::FetchChatHistoryItem& item : history.items) {
        sender.send_frame(WireMessage::encode_history_item(item.message.id, item.is_mine,
                                                                  item.message.body));
    }

    sender.send_frame(WireMessage::encode_history_end());
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
