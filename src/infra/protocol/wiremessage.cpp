#include "wiremessage.h"

#include <stdexcept>
#include <type_traits>
#include <utility>


namespace will {


namespace {


std::unique_ptr<WireMessageBase> from_variant(const WireMessage& message)
{
    return std::visit(
        [](const auto& typed_message) -> std::unique_ptr<WireMessageBase> {
            using T = std::decay_t<decltype(typed_message)>;
            if constexpr (std::is_same_v<T, UserChat>)
                return std::make_unique<UserChatMessage>(typed_message.body);
            else if constexpr (std::is_same_v<T, ServerReceiptAck>)
                return std::make_unique<ServerReceiptAckMessage>();
            else if constexpr (std::is_same_v<T, HistoryRequest>)
                return std::make_unique<HistoryRequestMessage>(typed_message.limit);
            else if constexpr (std::is_same_v<T, HistoryItemPayload>)
                return std::make_unique<HistoryItemMessage>(
                    typed_message.message_id, typed_message.is_mine, typed_message.body);
            else if constexpr (std::is_same_v<T, HistoryEnd>)
                return std::make_unique<HistoryEndMessage>();
            else if constexpr (std::is_same_v<T, LoginRequestPayload>)
                return std::make_unique<LoginRequestMessage>(typed_message.login, typed_message.password);
            else if constexpr (std::is_same_v<T, LoginResponsePayload>)
                return std::make_unique<LoginResponseMessage>(
                    typed_message.success, typed_message.token, typed_message.error_code);
            else if constexpr (std::is_same_v<T, BindToken>)
                return std::make_unique<BindTokenMessage>(typed_message.token);
            else if constexpr (std::is_same_v<T, AuthRequired>)
                return std::make_unique<AuthRequiredMessage>();
        },
        message);
}


WireMessage to_variant(const WireMessageBase& message)
{
    switch (message.type()) {
    case WireMessageType::UserChat: {
        const auto* chat = dynamic_cast<const UserChatMessage*>(&message);
        if (!chat)
            break;
        return UserChat{chat->body()};
    }
    case WireMessageType::ServerReceiptAck:
        return ServerReceiptAck{};
    case WireMessageType::HistoryRequest: {
        const auto* request = dynamic_cast<const HistoryRequestMessage*>(&message);
        if (!request)
            break;
        return HistoryRequest{request->limit()};
    }
    case WireMessageType::HistoryItem: {
        const auto* item = dynamic_cast<const HistoryItemMessage*>(&message);
        if (!item)
            break;
        return HistoryItemPayload{item->message_id(), item->is_mine(), item->body()};
    }
    case WireMessageType::HistoryEnd:
        return HistoryEnd{};
    case WireMessageType::LoginRequest: {
        const auto* login = dynamic_cast<const LoginRequestMessage*>(&message);
        if (!login)
            break;
        return LoginRequestPayload{login->login(), login->password()};
    }
    case WireMessageType::LoginResponse: {
        const auto* response = dynamic_cast<const LoginResponseMessage*>(&message);
        if (!response)
            break;
        return LoginResponsePayload{response->success(), response->token(), response->error_code()};
    }
    case WireMessageType::BindToken: {
        const auto* bind = dynamic_cast<const BindTokenMessage*>(&message);
        if (!bind)
            break;
        return BindToken{bind->token()};
    }
    case WireMessageType::AuthRequired:
        return AuthRequired{};
    }

    throw std::logic_error("to_variant: unknown WireMessageType");
}


} // namespace


std::vector<char> encode(const WireMessage& message) { return encode(*from_variant(message)); }


std::optional<WireMessage> decode(const std::vector<char>& payload)
{
    const auto message = decode_message(payload);
    if (!message)
        return std::nullopt;
    return to_variant(*message);
}


bool is_client_to_server(const WireMessage& message) noexcept
{
    return std::holds_alternative<UserChat>(message) || std::holds_alternative<HistoryRequest>(message)
           || std::holds_alternative<LoginRequestPayload>(message)
           || std::holds_alternative<BindToken>(message);
}


bool is_valid_client_to_server_payload(const std::vector<char>& payload) noexcept
{
    const auto message = decode(payload);
    if (!message)
        return false;

    if (!is_client_to_server(*message))
        return false;

    if (std::holds_alternative<UserChat>(*message))
        return true;

    if (const auto* request = std::get_if<HistoryRequest>(&*message))
        return request->limit >= 1u;

    return true;
}


std::string format_for_log(const WireMessage& message) { return format_for_log(*from_variant(message)); }


std::string format_for_log(const std::vector<char>& payload)
{
    if (payload.empty())
        return "<empty>";

    const auto message = decode_message(payload);
    if (!message)
        return "<unknown type=" + std::to_string(static_cast<unsigned int>(static_cast<unsigned char>(payload[0])))
               + " len=" + std::to_string(payload.size()) + ">";

    return message->format_for_log();
}


} // namespace will
