#include "wiremessage_client.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"

#include <memory>
#include <vector>


namespace will {


std::vector<char> encode(const WireMessageBase& message) { return message.encode(); }


std::unique_ptr<WireMessageBase> decode_message(const std::vector<char>& payload)
{
    if (payload.empty())
        return nullptr;

    switch (static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0]))) {
    case WireMessageType::UserChat:
        return UserChatMessage::from_bytes(payload);
    case WireMessageType::ServerReceiptAck:
        return ServerReceiptAckMessage::from_bytes(payload);
    case WireMessageType::HistoryRequest:
        return HistoryRequestMessage::from_bytes(payload);
    case WireMessageType::HistoryItem:
        return HistoryItemMessage::from_bytes(payload);
    case WireMessageType::HistoryEnd:
        return HistoryEndMessage::from_bytes(payload);
    case WireMessageType::LoginRequest:
        return LoginRequestMessage::from_bytes(payload);
    case WireMessageType::LoginResponse:
        return LoginResponseMessage::from_bytes(payload);
    case WireMessageType::BindToken:
        return BindTokenMessage::from_bytes(payload);
    case WireMessageType::AuthRequired:
        return AuthRequiredMessage::from_bytes(payload);
    }

    return nullptr;
}


std::unique_ptr<ClientMessage> decode_client_message(const std::vector<char>& payload)
{
    auto message = decode_message(payload);
    return std::unique_ptr<ClientMessage>(dynamic_cast<ClientMessage*>(message.release()));
}


std::unique_ptr<ServerMessage> decode_server_message(const std::vector<char>& payload)
{
    auto message = decode_message(payload);
    return std::unique_ptr<ServerMessage>(dynamic_cast<ServerMessage*>(message.release()));
}


std::string format_for_log(const WireMessageBase& message) { return message.format_for_log(); }


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


bool is_valid_client_to_server_payload(const std::vector<char>& payload) noexcept
{
    const auto message = decode_client_message(payload);
    if (!message)
        return false;

    if (const auto* request = dynamic_cast<const HistoryRequestMessage*>(message.get()))
        return request->limit() >= 1u;

    return true;
}


} // namespace will
