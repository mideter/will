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
        return static_cast<std::uint8_t>(LoginError::ExpiredToken);
    case domain::AuthResult::InvalidCredentials:
    default:
        return static_cast<std::uint8_t>(LoginError::InvalidCredentials);
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
    const auto message = decode(payload);
    if (!message || !is_client_to_server(*message)) {
        close_with_protocol_error(connection_id, "Protocol error: invalid client frame");
        return;
    }

    std::visit(
        [this, connection_id](const auto& typed_message) {
            using T = std::decay_t<decltype(typed_message)>;

            if constexpr (std::is_same_v<T, LoginRequestPayload>) {
                handle_login(connection_id, typed_message);
            } else if constexpr (std::is_same_v<T, BindToken>) {
                handle_bind_token(connection_id, typed_message);
            } else if constexpr (std::is_same_v<T, UserChat>) {
                if (!account_store_.has(connection_id)) {
                    send_auth_required(connection_id);
                    return;
                }
                handle_user_chat(connection_id, typed_message);
            } else if constexpr (std::is_same_v<T, HistoryRequest>) {
                if (!account_store_.has(connection_id)) {
                    send_auth_required(connection_id);
                    return;
                }
                handle_history_request(connection_id, typed_message);
            }
        },
        *message);
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


void ProtocolAdapter::handle_login(const std::uint64_t connection_id, const LoginRequestPayload& request)
{
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::AuthenticateUserInput input{request.login, request.password, now_ms};
    const auto outcome = authenticate_user_.execute(input);

    if (const auto* failure = std::get_if<domain::AuthResult>(&outcome)) {
        LoginResponsePayload response;
        response.success = false;
        response.error_code = login_error_code_for(*failure);
        send_payload(connection_id, encode(response));
        return;
    }

    const auto& success = std::get<domain::AuthenticateUserSuccess>(outcome);
    LoginResponsePayload response;
    response.success = true;
    response.token = success.account.session_token.value;
    send_payload(connection_id, encode(response));
}


void ProtocolAdapter::handle_bind_token(const std::uint64_t connection_id, const BindToken& token)
{
    const auto account = persistence_.sessions.resolve_token(domain::AuthToken{token.token});
    if (!account) {
        LoginResponsePayload response;
        response.success = false;
        response.error_code = static_cast<std::uint8_t>(LoginError::ExpiredToken);
        send_payload(connection_id, encode(response));
        return;
    }

    account_store_.set(connection_id, *account);
}


void ProtocolAdapter::send_auth_required(const std::uint64_t connection_id)
{
    send_payload(connection_id, encode(AuthRequired{}));
}


void ProtocolAdapter::handle_user_chat(const std::uint64_t connection_id, const UserChat& chat)
{
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();

    const domain::SendChatMessageInput input{
        *account_store_.get(connection_id),
        domain::ParticipantId{connection_id},
        domain::ChatId::global(),
        chat.body,
        now_ms,
    };

    (void)send_chat_message_.execute(input);
    send_payload(connection_id, encode(ServerReceiptAck{}));
}


void ProtocolAdapter::handle_history_request(const std::uint64_t connection_id, const HistoryRequest& request)
{
    const domain::FetchChatHistoryInput input{*account_store_.get(connection_id), domain::ChatId::global(),
                                              request.limit};

    const auto outcome = fetch_chat_history_.execute(input);
    if (const auto* error = std::get_if<domain::DomainError>(&outcome)) {
        (void)error;
        close_with_protocol_error(connection_id, "Protocol error: invalid HistoryRequest");
        return;
    }

    const auto& history = std::get<domain::FetchChatHistoryResult>(outcome);
    for (const domain::FetchChatHistoryItem& item : history.items) {
        HistoryItemPayload item_payload;
        item_payload.message_id = item.message.id;
        item_payload.is_mine = item.is_mine;
        item_payload.body = item.message.body;
        send_payload(connection_id, encode(item_payload));
    }

    send_payload(connection_id, encode(HistoryEnd{}));
}


std::vector<char> ProtocolAdapter::encode_user_chat(const std::string_view utf8_body)
{
    return encode(UserChat{std::string(utf8_body)});
}


std::string ProtocolAdapter::format_payload_for_log(const std::vector<char>& payload)
{
    return format_for_log(payload);
}


} // namespace will
