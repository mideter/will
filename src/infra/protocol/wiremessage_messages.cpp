#include "wiremessage_client.h"
#include "wiremessage_codec_internal.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"

#include <stdexcept>


namespace will {


// --- UserChatMessage ---

UserChatMessage::UserChatMessage(std::string body) : body_(std::move(body)) {}


UserChatMessage::UserChatMessage(std::string name, std::string body)
    : name_(std::move(name))
    , body_(std::move(body))
{}


WireMessage::Type UserChatMessage::type() const noexcept { return WireMessage::Type::UserChat; }


std::vector<char> UserChatMessage::encode() const
{
    const std::size_t total = 1u + 4u + name_.size() + body_.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessage::Type::encode_user_chat: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out;
    out.reserve(total);
    out.push_back(static_cast<char>(WireMessage::Type::UserChat));
    WireMessageCodec::Internal::append_length_prefixed_string(out, name_);
    if (!body_.empty())
        out.insert(out.end(), body_.begin(), body_.end());
    return out;
}


std::string UserChatMessage::format_for_log() const
{
    return WireMessageCodec::Internal::format_user_chat_body_for_log(body_);
}


std::unique_ptr<UserChatMessage> UserChatMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::UserChat)
        return nullptr;

    std::size_t offset = 1u;
    std::string_view name;
    if (!WireMessageCodec::Internal::read_length_prefixed_string_allow_empty(name, payload, offset))
        return nullptr;

    auto message = std::make_unique<UserChatMessage>();
    message->name_.assign(name);
    message->body_.assign(payload.begin() + static_cast<std::ptrdiff_t>(offset), payload.end());
    return message;
}


bool UserChatMessage::operator==(const UserChatMessage& other) const noexcept
{
    return name_ == other.name_ && body_ == other.body_;
}


// --- BindTokenMessage ---

BindTokenMessage::BindTokenMessage(std::string token) : token_(std::move(token)) {}


WireMessage::Type BindTokenMessage::type() const noexcept { return WireMessage::Type::BindToken; }


std::vector<char> BindTokenMessage::encode() const
{
    if (token_.empty())
        throw std::runtime_error("WireMessage::Type::encode_bind_token: token required");

    std::vector<char> out;
    out.reserve(1u + 4u + token_.size());
    out.push_back(static_cast<char>(WireMessage::Type::BindToken));
    WireMessageCodec::Internal::append_length_prefixed_string(out, token_);
    return out;
}


std::string BindTokenMessage::format_for_log() const
{
    return "BindToken(len=" + std::to_string(token_.size()) + ')';
}


std::unique_ptr<BindTokenMessage> BindTokenMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::BindToken)
        return nullptr;

    std::size_t offset = 1u;
    std::string_view token;
    if (!WireMessageCodec::Internal::read_length_prefixed_string(token, payload, offset)
        || offset != payload.size())
        return nullptr;

    return std::make_unique<BindTokenMessage>(std::string(token));
}


bool BindTokenMessage::operator==(const BindTokenMessage& other) const noexcept
{
    return token_ == other.token_;
}


// --- HistoryRequestMessage ---

HistoryRequestMessage::HistoryRequestMessage(std::uint32_t limit) : limit_(limit) {}


WireMessage::Type HistoryRequestMessage::type() const noexcept { return WireMessage::Type::HistoryRequest; }


std::vector<char> HistoryRequestMessage::encode() const
{
    if (limit_ < 1u)
        throw std::runtime_error("WireMessage::Type::encode_history_request: limit must be at least 1");

    std::vector<char> out;
    out.reserve(5);
    out.push_back(static_cast<char>(WireMessage::Type::HistoryRequest));
    WireMessageCodec::Internal::append_u32_be(out, limit_);
    return out;
}


std::string HistoryRequestMessage::format_for_log() const
{
    return "HistoryRequest(" + std::to_string(limit_) + ')';
}


std::unique_ptr<HistoryRequestMessage> HistoryRequestMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::HistoryRequest
        || payload.size() != 5u)
        return nullptr;

    const auto limit = WireMessageCodec::Internal::read_u32_be_at(
        reinterpret_cast<const unsigned char*>(payload.data() + 1));
    if (limit < 1u)
        return nullptr;

    return std::make_unique<HistoryRequestMessage>(limit);
}


bool HistoryRequestMessage::operator==(const HistoryRequestMessage& other) const noexcept
{
    return limit_ == other.limit_;
}


// --- AuthRequiredMessage ---

WireMessage::Type AuthRequiredMessage::type() const noexcept { return WireMessage::Type::AuthRequired; }


std::vector<char> AuthRequiredMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessage::Type::AuthRequired)};
}


std::string AuthRequiredMessage::format_for_log() const { return "AuthRequired"; }


std::unique_ptr<AuthRequiredMessage> AuthRequiredMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::AuthRequired)
        return nullptr;

    return std::make_unique<AuthRequiredMessage>();
}


// --- AuthOkMessage ---

WireMessage::Type AuthOkMessage::type() const noexcept { return WireMessage::Type::AuthOk; }


std::vector<char> AuthOkMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessage::Type::AuthOk)};
}


std::string AuthOkMessage::format_for_log() const { return "AuthOk"; }


std::unique_ptr<AuthOkMessage> AuthOkMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::AuthOk)
        return nullptr;

    return std::make_unique<AuthOkMessage>();
}


// --- ServerReceiptAckMessage ---

WireMessage::Type ServerReceiptAckMessage::type() const noexcept
{
    return WireMessage::Type::ServerReceiptAck;
}


std::vector<char> ServerReceiptAckMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessage::Type::ServerReceiptAck)};
}


std::string ServerReceiptAckMessage::format_for_log() const { return "ServerReceiptAck"; }


std::unique_ptr<ServerReceiptAckMessage> ServerReceiptAckMessage::from_bytes(
    const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0]))
               != WireMessage::Type::ServerReceiptAck)
        return nullptr;

    return std::make_unique<ServerReceiptAckMessage>();
}


// --- HistoryItemMessage ---

HistoryItemMessage::HistoryItemMessage(std::uint64_t message_id, bool is_mine, std::string name, std::string body)
    : message_id_(message_id)
    , is_mine_(is_mine)
    , name_(std::move(name))
    , body_(std::move(body))
{}


WireMessage::Type HistoryItemMessage::type() const noexcept { return WireMessage::Type::HistoryItem; }


std::vector<char> HistoryItemMessage::encode() const
{
    const std::size_t total = 1u + 8u + 1u + 4u + name_.size() + 4u + body_.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessage::Type::encode_history_item: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out;
    out.reserve(total);
    out.push_back(static_cast<char>(WireMessage::Type::HistoryItem));
    WireMessageCodec::Internal::append_u64_be(out, message_id_);
    out.push_back(is_mine_ ? '\1' : '\0');
    WireMessageCodec::Internal::append_length_prefixed_string(out, name_);
    WireMessageCodec::Internal::append_u32_be(out, static_cast<std::uint32_t>(body_.size()));
    if (!body_.empty())
        out.insert(out.end(), body_.begin(), body_.end());
    return out;
}


std::string HistoryItemMessage::format_for_log() const
{
    std::string out = "HistoryItem(id=";
    out += std::to_string(message_id_);
    out += is_mine_ ? ", mine" : ", peer";
    out += ", name=";
    out += name_;
    out += ", ";
    out += std::to_string(body_.size());
    out += " bytes)";
    return out;
}


std::unique_ptr<HistoryItemMessage> HistoryItemMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::HistoryItem
        || payload.size() < 14u)
        return nullptr;

    const auto* data = reinterpret_cast<const unsigned char*>(payload.data());
    const std::uint64_t message_id = WireMessageCodec::Internal::read_u64_be(data + 1);
    const bool is_mine = data[9] != 0u;

    std::size_t offset = 10u;
    std::string_view name;
    if (!WireMessageCodec::Internal::read_length_prefixed_string_allow_empty(name, payload, offset))
        return nullptr;

    if (offset + 4u > payload.size())
        return nullptr;
    const std::uint32_t body_len =
        WireMessageCodec::Internal::read_u32_be_at(reinterpret_cast<const unsigned char*>(payload.data()) + offset);
    offset += 4u;
    if (offset + body_len != payload.size())
        return nullptr;

    auto item = std::make_unique<HistoryItemMessage>();
    item->message_id_ = message_id;
    item->is_mine_ = is_mine;
    item->name_.assign(name);
    item->body_.assign(payload.begin() + static_cast<std::ptrdiff_t>(offset), payload.end());
    return item;
}


bool HistoryItemMessage::operator==(const HistoryItemMessage& other) const noexcept
{
    return message_id_ == other.message_id_ && is_mine_ == other.is_mine_ && name_ == other.name_
        && body_ == other.body_;
}


// --- HistoryEndMessage ---

WireMessage::Type HistoryEndMessage::type() const noexcept { return WireMessage::Type::HistoryEnd; }


std::vector<char> HistoryEndMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessage::Type::HistoryEnd)};
}


std::string HistoryEndMessage::format_for_log() const { return "HistoryEnd"; }


std::unique_ptr<HistoryEndMessage> HistoryEndMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::HistoryEnd)
        return nullptr;

    return std::make_unique<HistoryEndMessage>();
}


// --- PingMessage ---

WireMessage::Type PingMessage::type() const noexcept { return WireMessage::Type::Ping; }


std::vector<char> PingMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessage::Type::Ping)};
}


std::string PingMessage::format_for_log() const { return "Ping"; }


std::unique_ptr<PingMessage> PingMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::Ping)
        return nullptr;

    return std::make_unique<PingMessage>();
}


// --- PongMessage ---

WireMessage::Type PongMessage::type() const noexcept { return WireMessage::Type::Pong; }


std::vector<char> PongMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessage::Type::Pong)};
}


std::string PongMessage::format_for_log() const { return "Pong"; }


std::unique_ptr<PongMessage> PongMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::Pong)
        return nullptr;

    return std::make_unique<PongMessage>();
}


} // namespace will
