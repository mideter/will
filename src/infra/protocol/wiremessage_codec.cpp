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


std::string format_for_log(const WireMessageBase& message) { return message.format_for_log(); }


} // namespace will
