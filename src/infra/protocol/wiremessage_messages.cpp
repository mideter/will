#include "wiremessage_client.h"
#include "wiremessage_codec_internal.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"

#include "support/phone_number.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>


namespace will {


// --- UserChatMessage ---

UserChatMessage::UserChatMessage(std::string body) : body_(std::move(body)) {}


WireMessage::Type UserChatMessage::type() const noexcept { return WireMessage::Type::UserChat; }


std::vector<char> UserChatMessage::encode() const
{
    const std::string_view utf8_body = body_;
    const std::size_t total = 1u + utf8_body.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessage::Type::encode_user_chat: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out(total);
    out[0] = static_cast<char>(WireMessage::Type::UserChat);
    if (!utf8_body.empty())
        std::memcpy(out.data() + 1, utf8_body.data(), utf8_body.size());
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

    auto message = std::make_unique<UserChatMessage>();
    message->body_.assign(payload.begin() + 1, payload.end());
    return message;
}


bool UserChatMessage::operator==(const UserChatMessage& other) const noexcept
{
    return body_ == other.body_;
}


namespace {


bool is_valid_otp_code(std::string_view code) noexcept
{
    if (code.size() < 4u || code.size() > 8u)
        return false;
    return std::all_of(code.begin(), code.end(),
                       [](const char c) noexcept { return c >= '0' && c <= '9'; });
}


} // namespace


// --- OtpPhoneRequestMessage ---

OtpPhoneRequestMessage::OtpPhoneRequestMessage(std::string phone_e164) : phone_e164_(std::move(phone_e164)) {}


WireMessage::Type OtpPhoneRequestMessage::type() const noexcept
{
    return WireMessage::Type::OtpPhoneRequest;
}


std::vector<char> OtpPhoneRequestMessage::encode() const
{
    const auto parsed = domain::PhoneNumber::parse(phone_e164_);

    if (!parsed)
        throw std::runtime_error("WireMessage::Type::encode_otp_phone_request: invalid E.164 phone");

    const std::string_view utf8_phone = parsed->e164();
    const std::size_t total = 1u + utf8_phone.size();

    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessage::Type::encode_otp_phone_request: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out(total);
    out[0] = static_cast<char>(WireMessage::Type::OtpPhoneRequest);

    if (!utf8_phone.empty())
        std::memcpy(out.data() + 1, utf8_phone.data(), utf8_phone.size());
    
    return out;
}


std::string OtpPhoneRequestMessage::format_for_log() const
{
    return "OtpPhoneRequest(phone=" + phone_e164_ + ')';
}


std::unique_ptr<OtpPhoneRequestMessage> OtpPhoneRequestMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0]))
               != WireMessage::Type::OtpPhoneRequest)
        return nullptr;

    const std::string_view raw_phone{payload.data() + 1, payload.size() - 1};
    const auto parsed = domain::PhoneNumber::parse(raw_phone);
    if (!parsed)
        return nullptr;

    return std::make_unique<OtpPhoneRequestMessage>(parsed->e164());
}


bool OtpPhoneRequestMessage::operator==(const OtpPhoneRequestMessage& other) const noexcept
{
    return phone_e164_ == other.phone_e164_;
}


// --- OtpCodeSubmitMessage ---

OtpCodeSubmitMessage::OtpCodeSubmitMessage(std::string code) : code_(std::move(code)) {}


WireMessage::Type OtpCodeSubmitMessage::type() const noexcept { return WireMessage::Type::OtpCodeSubmit; }


std::vector<char> OtpCodeSubmitMessage::encode() const
{
    if (!is_valid_otp_code(code_))
        throw std::runtime_error("WireMessage::Type::encode_otp_code_submit: code must be 4-8 ASCII digits");

    const std::string_view ascii_code = code_;
    const std::size_t total = 1u + ascii_code.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessage::Type::encode_otp_code_submit: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out(total);
    out[0] = static_cast<char>(WireMessage::Type::OtpCodeSubmit);
    if (!ascii_code.empty())
        std::memcpy(out.data() + 1, ascii_code.data(), ascii_code.size());
    return out;
}


std::string OtpCodeSubmitMessage::format_for_log() const { return "OtpCodeSubmit(len=" + std::to_string(code_.size()) + ')'; }


std::unique_ptr<OtpCodeSubmitMessage> OtpCodeSubmitMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::OtpCodeSubmit)
        return nullptr;

    const std::string_view code{payload.data() + 1, payload.size() - 1};
    if (!is_valid_otp_code(code))
        return nullptr;

    return std::make_unique<OtpCodeSubmitMessage>(std::string(code));
}


bool OtpCodeSubmitMessage::operator==(const OtpCodeSubmitMessage& other) const noexcept
{
    return code_ == other.code_;
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


// --- OtpSentMessage ---

WireMessage::Type OtpSentMessage::type() const noexcept { return WireMessage::Type::OtpSent; }


std::vector<char> OtpSentMessage::encode() const
{
    return std::vector<char>{static_cast<char>(WireMessage::Type::OtpSent)};
}


std::string OtpSentMessage::format_for_log() const { return "OtpSent"; }


std::unique_ptr<OtpSentMessage> OtpSentMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.size() != 1u
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::OtpSent)
        return nullptr;

    return std::make_unique<OtpSentMessage>();
}


// --- OtpVerifyResponseMessage ---

OtpVerifyResponseMessage::OtpVerifyResponseMessage(bool success, std::string token, std::uint8_t error_code)
    : success_(success), token_(std::move(token)), error_code_(error_code)
{
}


WireMessage::Type OtpVerifyResponseMessage::type() const noexcept
{
    return WireMessage::Type::OtpVerifyResponse;
}


std::vector<char> OtpVerifyResponseMessage::encode() const
{
    if (success_) {
        if (token_.empty())
            throw std::runtime_error("WireMessage::Type::encode_otp_verify_response_success: token required");

        std::vector<char> out;
        out.reserve(1u + 1u + 4u + token_.size());
        out.push_back(static_cast<char>(WireMessage::Type::OtpVerifyResponse));
        out.push_back('\1');
        WireMessageCodec::Internal::append_length_prefixed_string(out, token_);
        return out;
    }

    if (error_code_ == 0u)
        throw std::runtime_error("WireMessage::Type::encode_otp_verify_response_failure: error_code required");

    return std::vector<char>{static_cast<char>(WireMessage::Type::OtpVerifyResponse), '\0',
                             static_cast<char>(error_code_)};
}


std::string OtpVerifyResponseMessage::format_for_log() const
{
    if (success_)
        return "OtpVerifyResponse(ok, token_len=" + std::to_string(token_.size()) + ')';
    return "OtpVerifyResponse(error=" + std::to_string(error_code_) + ')';
}


std::unique_ptr<OtpVerifyResponseMessage> OtpVerifyResponseMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0]))
               != WireMessage::Type::OtpVerifyResponse
        || payload.size() < 2u)
        return nullptr;

    const bool success = static_cast<unsigned char>(payload[1]) != 0u;

    if (success) {
        std::size_t offset = 2u;
        std::string_view token;
        if (!WireMessageCodec::Internal::read_length_prefixed_string(token, payload, offset)
            || offset != payload.size())
            return nullptr;
        return std::make_unique<OtpVerifyResponseMessage>(true, std::string(token), 0);
    }

    if (payload.size() != 3u || payload[2] == '\0')
        return nullptr;

    return std::make_unique<OtpVerifyResponseMessage>(false, std::string{},
                                                      static_cast<std::uint8_t>(payload[2]));
}


bool OtpVerifyResponseMessage::operator==(const OtpVerifyResponseMessage& other) const noexcept
{
    return success_ == other.success_ && token_ == other.token_ && error_code_ == other.error_code_;
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

HistoryItemMessage::HistoryItemMessage(std::uint64_t message_id, bool is_mine, std::string body)
    : message_id_(message_id), is_mine_(is_mine), body_(std::move(body))
{
}


WireMessage::Type HistoryItemMessage::type() const noexcept { return WireMessage::Type::HistoryItem; }


std::vector<char> HistoryItemMessage::encode() const
{
    const std::string_view utf8_body = body_;
    const std::size_t total = 1u + 8u + 1u + 4u + utf8_body.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessage::Type::encode_history_item: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out;
    out.reserve(total);
    out.push_back(static_cast<char>(WireMessage::Type::HistoryItem));
    WireMessageCodec::Internal::append_u64_be(out, message_id_);
    out.push_back(is_mine_ ? '\1' : '\0');
    WireMessageCodec::Internal::append_u32_be(out, static_cast<std::uint32_t>(utf8_body.size()));
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


std::unique_ptr<HistoryItemMessage> HistoryItemMessage::from_bytes(const std::vector<char>& payload)
{
    if (payload.empty()
        || static_cast<WireMessage::Type>(static_cast<std::uint8_t>(payload[0])) != WireMessage::Type::HistoryItem
        || payload.size() < 14u)
        return nullptr;

    const auto* data = reinterpret_cast<const unsigned char*>(payload.data());
    const std::uint64_t message_id = WireMessageCodec::Internal::read_u64_be(data + 1);
    const bool is_mine = data[9] != 0u;
    const std::uint32_t body_len = WireMessageCodec::Internal::read_u32_be_at(data + 10);

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


} // namespace will
