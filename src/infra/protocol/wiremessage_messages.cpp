#include "wiremessage_client.h"
#include "wiremessage_codec_internal.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"

#include <cstring>
#include <stdexcept>


namespace will {


namespace {


using namespace wiremessage_codec;


} // namespace


// --- UserChatMessage ---

UserChatMessage::UserChatMessage(std::string body) : body_(std::move(body)) {}


WireMessageType UserChatMessage::type() const noexcept { return WireMessageType::UserChat; }


std::vector<char> UserChatMessage::encode() const
{
    const std::string_view utf8_body = body_;
    const std::size_t total = 1u + utf8_body.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessageType::encode_user_chat: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out(total);
    out[0] = static_cast<char>(WireMessageType::UserChat);
    if (!utf8_body.empty())
        std::memcpy(out.data() + 1, utf8_body.data(), utf8_body.size());
    return out;
}


std::string UserChatMessage::format_for_log() const { return format_user_chat_body_for_log(body_); }


void UserChatMessage::accept(ClientMessageVisitor& visitor) const { visitor.on(*this); }


void UserChatMessage::accept(ServerMessageVisitor& visitor) const { visitor.on(*this); }


std::unique_ptr<UserChatMessage> UserChatMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::UserChat)
        return nullptr;

    auto message = std::make_unique<UserChatMessage>();
    message->body_.assign(payload.begin() + 1, payload.end());
    return message;
}


bool UserChatMessage::operator==(const UserChatMessage& other) const noexcept
{
    return body_ == other.body_;
}


// --- LoginRequestMessage ---

LoginRequestMessage::LoginRequestMessage(std::string login, std::string password)
    : login_(std::move(login)), password_(std::move(password))
{
}


WireMessageType LoginRequestMessage::type() const noexcept { return WireMessageType::LoginRequest; }


std::vector<char> LoginRequestMessage::encode() const
{
    if (login_.empty() || password_.empty())
        throw std::runtime_error("WireMessageType::encode_login_request: login and password required");

    std::vector<char> out;
    out.reserve(1u + 8u + login_.size() + password_.size());
    out.push_back(static_cast<char>(WireMessageType::LoginRequest));
    append_length_prefixed_string(out, login_);
    append_length_prefixed_string(out, password_);

    if (out.size() > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessageType::encode_login_request: payload exceeds TcpFrame::MaxPayloadBytes");

    return out;
}


std::string LoginRequestMessage::format_for_log() const { return "LoginRequest(login=" + login_ + ')'; }


void LoginRequestMessage::accept(ClientMessageVisitor& visitor) const { visitor.on(*this); }


std::unique_ptr<LoginRequestMessage> LoginRequestMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::LoginRequest)
        return nullptr;

    std::size_t offset = 1u;
    std::string_view login;
    std::string_view password;
    if (!read_length_prefixed_string(login, payload, offset))
        return nullptr;
    if (!read_length_prefixed_string(password, payload, offset))
        return nullptr;
    if (offset != payload.size())
        return nullptr;

    return std::make_unique<LoginRequestMessage>(std::string(login), std::string(password));
}


bool LoginRequestMessage::operator==(const LoginRequestMessage& other) const noexcept
{
    return login_ == other.login_ && password_ == other.password_;
}


// --- BindTokenMessage ---

BindTokenMessage::BindTokenMessage(std::string token) : token_(std::move(token)) {}


WireMessageType BindTokenMessage::type() const noexcept { return WireMessageType::BindToken; }


std::vector<char> BindTokenMessage::encode() const
{
    if (token_.empty())
        throw std::runtime_error("WireMessageType::encode_bind_token: token required");

    std::vector<char> out;
    out.reserve(1u + 4u + token_.size());
    out.push_back(static_cast<char>(WireMessageType::BindToken));
    append_length_prefixed_string(out, token_);
    return out;
}


std::string BindTokenMessage::format_for_log() const
{
    return "BindToken(len=" + std::to_string(token_.size()) + ')';
}


void BindTokenMessage::accept(ClientMessageVisitor& visitor) const { visitor.on(*this); }


std::unique_ptr<BindTokenMessage> BindTokenMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::BindToken)
        return nullptr;

    std::size_t offset = 1u;
    std::string_view token;
    if (!read_length_prefixed_string(token, payload, offset) || offset != payload.size())
        return nullptr;

    return std::make_unique<BindTokenMessage>(std::string(token));
}


bool BindTokenMessage::operator==(const BindTokenMessage& other) const noexcept
{
    return token_ == other.token_;
}


// --- HistoryRequestMessage ---

HistoryRequestMessage::HistoryRequestMessage(std::uint32_t limit) : limit_(limit) {}


WireMessageType HistoryRequestMessage::type() const noexcept { return WireMessageType::HistoryRequest; }


std::vector<char> HistoryRequestMessage::encode() const
{
    if (limit_ < 1u)
        throw std::runtime_error("WireMessageType::encode_history_request: limit must be at least 1");

    std::vector<char> out;
    out.reserve(5);
    out.push_back(static_cast<char>(WireMessageType::HistoryRequest));
    append_u32_be(out, limit_);
    return out;
}


std::string HistoryRequestMessage::format_for_log() const
{
    return "HistoryRequest(" + std::to_string(limit_) + ')';
}


void HistoryRequestMessage::accept(ClientMessageVisitor& visitor) const { visitor.on(*this); }


std::unique_ptr<HistoryRequestMessage> HistoryRequestMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::HistoryRequest
        || payload.size() != 5u)
        return nullptr;

    const auto limit = read_u32_be_at(reinterpret_cast<const unsigned char*>(payload.data() + 1));
    if (limit < 1u)
        return nullptr;

    return std::make_unique<HistoryRequestMessage>(limit);
}


bool HistoryRequestMessage::operator==(const HistoryRequestMessage& other) const noexcept
{
    return limit_ == other.limit_;
}


// --- LoginResponseMessage ---

LoginResponseMessage::LoginResponseMessage(bool success, std::string token, std::uint8_t error_code)
    : success_(success), token_(std::move(token)), error_code_(error_code)
{
}


WireMessageType LoginResponseMessage::type() const noexcept { return WireMessageType::LoginResponse; }


std::vector<char> LoginResponseMessage::encode() const
{
    if (success_) {
        if (token_.empty())
            throw std::runtime_error("WireMessageType::encode_login_response_success: token required");

        std::vector<char> out;
        out.reserve(1u + 1u + 4u + token_.size());
        out.push_back(static_cast<char>(WireMessageType::LoginResponse));
        out.push_back('\1');
        append_length_prefixed_string(out, token_);
        return out;
    }

    if (error_code_ == 0u)
        throw std::runtime_error("WireMessageType::encode_login_response_failure: error_code required");

    return std::vector<char>{static_cast<char>(WireMessageType::LoginResponse), '\0',
                             static_cast<char>(error_code_)};
}


std::string LoginResponseMessage::format_for_log() const
{
    if (success_)
        return "LoginResponse(ok, token_len=" + std::to_string(token_.size()) + ')';
    return "LoginResponse(error=" + std::to_string(error_code_) + ')';
}


void LoginResponseMessage::accept(ServerMessageVisitor& visitor) const { visitor.on(*this); }


std::unique_ptr<LoginResponseMessage> LoginResponseMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::LoginResponse
        || payload.size() < 2u)
        return nullptr;

    const bool success = static_cast<unsigned char>(payload[1]) != 0u;

    if (success) {
        std::size_t offset = 2u;
        std::string_view token;
        if (!read_length_prefixed_string(token, payload, offset) || offset != payload.size())
            return nullptr;
        return std::make_unique<LoginResponseMessage>(true, std::string(token), 0);
    }

    if (payload.size() != 3u || payload[2] == '\0')
        return nullptr;

    return std::make_unique<LoginResponseMessage>(false, std::string{},
                                                  static_cast<std::uint8_t>(payload[2]));
}


bool LoginResponseMessage::operator==(const LoginResponseMessage& other) const noexcept
{
    return success_ == other.success_ && token_ == other.token_ && error_code_ == other.error_code_;
}


// --- AuthRequiredMessage ---

WireMessageType AuthRequiredMessage::type() const noexcept { return WireMessageType::AuthRequired; }


std::vector<char> AuthRequiredMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessageType::AuthRequired)};
}


std::string AuthRequiredMessage::format_for_log() const { return "AuthRequired"; }


void AuthRequiredMessage::accept(ServerMessageVisitor& visitor) const { visitor.on(*this); }


std::unique_ptr<AuthRequiredMessage> AuthRequiredMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::AuthRequired)
        return nullptr;

    return std::make_unique<AuthRequiredMessage>();
}


// --- ServerReceiptAckMessage ---

WireMessageType ServerReceiptAckMessage::type() const noexcept
{
    return WireMessageType::ServerReceiptAck;
}


std::vector<char> ServerReceiptAckMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessageType::ServerReceiptAck)};
}


std::string ServerReceiptAckMessage::format_for_log() const { return "ServerReceiptAck"; }


void ServerReceiptAckMessage::accept(ServerMessageVisitor& visitor) const { visitor.on(*this); }


std::unique_ptr<ServerReceiptAckMessage> ServerReceiptAckMessage::from_bytes(
    const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0]))
               != WireMessageType::ServerReceiptAck)
        return nullptr;

    return std::make_unique<ServerReceiptAckMessage>();
}


// --- HistoryItemMessage ---

HistoryItemMessage::HistoryItemMessage(std::uint64_t message_id, bool is_mine, std::string body)
    : message_id_(message_id), is_mine_(is_mine), body_(std::move(body))
{
}


WireMessageType HistoryItemMessage::type() const noexcept { return WireMessageType::HistoryItem; }


std::vector<char> HistoryItemMessage::encode() const
{
    const std::string_view utf8_body = body_;
    const std::size_t total = 1u + 8u + 1u + 4u + utf8_body.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessageType::encode_history_item: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out;
    out.reserve(total);
    out.push_back(static_cast<char>(WireMessageType::HistoryItem));
    append_u64_be(out, message_id_);
    out.push_back(is_mine_ ? '\1' : '\0');
    append_u32_be(out, static_cast<std::uint32_t>(utf8_body.size()));
    if (!utf8_body.empty())
        out.insert(out.end(), utf8_body.begin(), utf8_body.end());
    return out;
}


std::string HistoryItemMessage::format_for_log() const
{
    std::string out = "HistoryItem(id=";
    out += std::to_string(message_id_);
    out += is_mine_ ? ", mine" : ", peer";
    out += ", ";
    out += std::to_string(body_.size());
    out += " bytes)";
    return out;
}


void HistoryItemMessage::accept(ServerMessageVisitor& visitor) const { visitor.on(*this); }


std::unique_ptr<HistoryItemMessage> HistoryItemMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::HistoryItem
        || payload.size() < 14u)
        return nullptr;

    const auto* data = reinterpret_cast<const unsigned char*>(payload.data());
    const std::uint64_t message_id = read_u64_be(data + 1);
    const bool is_mine = data[9] != 0u;
    const std::uint32_t body_len = read_u32_be_at(data + 10);

    if (14u + body_len != payload.size())
        return nullptr;

    auto item = std::make_unique<HistoryItemMessage>();
    item->message_id_ = message_id;
    item->is_mine_ = is_mine;
    item->body_.assign(payload.begin() + 14, payload.end());
    return item;
}


bool HistoryItemMessage::operator==(const HistoryItemMessage& other) const noexcept
{
    return message_id_ == other.message_id_ && is_mine_ == other.is_mine_ && body_ == other.body_;
}


// --- HistoryEndMessage ---

WireMessageType HistoryEndMessage::type() const noexcept { return WireMessageType::HistoryEnd; }


std::vector<char> HistoryEndMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessageType::HistoryEnd)};
}


std::string HistoryEndMessage::format_for_log() const { return "HistoryEnd"; }


void HistoryEndMessage::accept(ServerMessageVisitor& visitor) const { visitor.on(*this); }


std::unique_ptr<HistoryEndMessage> HistoryEndMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::HistoryEnd)
        return nullptr;

    return std::make_unique<HistoryEndMessage>();
}


} // namespace will
