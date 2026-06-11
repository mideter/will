#include "wiremessage_codec.h"

#include "wiremessage_client.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"

#include <memory>
#include <vector>


namespace will {


std::vector<char> WireMessageCodec::encode(const WireMessage& message) { return message.encode(); }


std::unique_ptr<WireMessage> WireMessageCodec::decode(const std::vector<char>& payload)
{
    if (payload.empty())
        return nullptr;

    switch (static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0]))) {
    case WireMessage::Type::UserChat:
        return UserChatMessage::from_bytes(payload);
    case WireMessage::Type::ServerReceiptAck:
        return ServerReceiptAckMessage::from_bytes(payload);
    case WireMessage::Type::HistoryRequest:
        return HistoryRequestMessage::from_bytes(payload);
    case WireMessage::Type::HistoryItem:
        return HistoryItemMessage::from_bytes(payload);
    case WireMessage::Type::HistoryEnd:
        return HistoryEndMessage::from_bytes(payload);
    case WireMessage::Type::LoginRequest:
        return LoginRequestMessage::from_bytes(payload);
    case WireMessage::Type::LoginResponse:
        return LoginResponseMessage::from_bytes(payload);
    case WireMessage::Type::BindToken:
        return BindTokenMessage::from_bytes(payload);
    case WireMessage::Type::AuthRequired:
        return AuthRequiredMessage::from_bytes(payload);
    }

    return nullptr;
}


std::unique_ptr<ClientMessage> WireMessageCodec::decode_client(const std::vector<char>& payload)
{
    auto message = decode_message(payload);
    return std::unique_ptr<ClientMessage>(dynamic_cast<ClientMessage*>(message.release()));
}


std::unique_ptr<ServerMessage> WireMessageCodec::decode_server(const std::vector<char>& payload)
{
    auto message = decode(payload);
    return std::unique_ptr<ServerMessage>(dynamic_cast<ServerMessage*>(message.release()));
}


std::string WireMessageCodec::format_for_log(const std::vector<char>& payload)
{
    if (payload.empty())
        return "<empty>";

    const auto message = decode(payload);
    if (!message)
        return "<unknown type=" + std::to_string(static_cast<unsigned int>(static_cast<unsigned char>(payload[0])))
               + " len=" + std::to_string(payload.size()) + ">";

    return message->format_for_log();
}


} // namespace will
