#include "wiremessage.h"

#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>


namespace will {


namespace {


void append_u32_be(std::vector<char>& out, const std::uint32_t value)
{
    const auto bytes = TcpFrame::u32_be(value);
    out.insert(out.end(), reinterpret_cast<const char*>(bytes.data()),
               reinterpret_cast<const char*>(bytes.data()) + bytes.size());
}


std::uint32_t read_u32_be_at(const unsigned char* data) noexcept
{
    std::array<unsigned char, 4> buf{};
    std::memcpy(buf.data(), data, buf.size());
    return TcpFrame::read_u32_be(buf);
}


void append_u64_be(std::vector<char>& out, std::uint64_t value)
{
    for (int shift = 56; shift >= 0; shift -= 8)
        out.push_back(static_cast<char>((value >> shift) & 0xffu));
}


std::uint64_t read_u64_be(const unsigned char* data) noexcept
{
    std::uint64_t value = 0;
    for (int i = 0; i < 8; ++i)
        value = (value << 8u) | std::uint64_t(data[i]);
    return value;
}


void append_length_prefixed_string(std::vector<char>& out, std::string_view value)
{
    if (value.size() > MaxAuthFieldBytes)
        throw std::runtime_error("WireMessage: auth field exceeds MaxAuthFieldBytes");

    append_u32_be(out, static_cast<std::uint32_t>(value.size()));
    if (!value.empty())
        out.insert(out.end(), value.begin(), value.end());
}


bool read_length_prefixed_string(std::string_view& field, const std::vector<char>& payload,
                                 std::size_t& offset)
{
    if (offset + 4u > payload.size())
        return false;

    const auto* data = reinterpret_cast<const unsigned char*>(payload.data());
    const std::uint32_t len = read_u32_be_at(data + offset);
    offset += 4u;

    if (len == 0u || len > MaxAuthFieldBytes || offset + len > payload.size())
        return false;

    field = std::string_view(payload.data() + offset, len);
    offset += len;
    return true;
}


std::vector<char> encode_user_chat_body(const UserChat& message)
{
    const std::string_view utf8_body = message.body;
    const std::size_t total = 1u + utf8_body.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessageType::encode_user_chat: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out(total);
    out[0] = static_cast<char>(WireMessageType::UserChat);
    if (!utf8_body.empty())
        std::memcpy(out.data() + 1, utf8_body.data(), utf8_body.size());
    return out;
}


std::vector<char> encode_server_receipt_ack_body(const ServerReceiptAck&)
{
    return std::vector<char>{static_cast<char>(WireMessageType::ServerReceiptAck)};
}


std::vector<char> encode_history_request_body(const HistoryRequest& message)
{
    if (message.limit < 1u)
        throw std::runtime_error("WireMessageType::encode_history_request: limit must be at least 1");

    std::vector<char> out;
    out.reserve(5);
    out.push_back(static_cast<char>(WireMessageType::HistoryRequest));
    append_u32_be(out, message.limit);
    return out;
}


std::vector<char> encode_history_item_body(const HistoryItemPayload& message)
{
    const std::string_view utf8_body = message.body;
    const std::size_t total = 1u + 8u + 1u + 4u + utf8_body.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessageType::encode_history_item: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out;
    out.reserve(total);
    out.push_back(static_cast<char>(WireMessageType::HistoryItem));
    append_u64_be(out, message.message_id);
    out.push_back(message.is_mine ? '\1' : '\0');
    append_u32_be(out, static_cast<std::uint32_t>(utf8_body.size()));
    if (!utf8_body.empty())
        out.insert(out.end(), utf8_body.begin(), utf8_body.end());
    return out;
}


std::vector<char> encode_history_end_body(const HistoryEnd&)
{
    return std::vector<char>{static_cast<char>(WireMessageType::HistoryEnd)};
}


std::vector<char> encode_login_request_body(const LoginRequestPayload& message)
{
    if (message.login.empty() || message.password.empty())
        throw std::runtime_error("WireMessageType::encode_login_request: login and password required");

    std::vector<char> out;
    out.reserve(1u + 8u + message.login.size() + message.password.size());
    out.push_back(static_cast<char>(WireMessageType::LoginRequest));
    append_length_prefixed_string(out, message.login);
    append_length_prefixed_string(out, message.password);

    if (out.size() > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WireMessageType::encode_login_request: payload exceeds TcpFrame::MaxPayloadBytes");

    return out;
}


std::vector<char> encode_login_response_body(const LoginResponsePayload& message)
{
    if (message.success) {
        if (message.token.empty())
            throw std::runtime_error("WireMessageType::encode_login_response_success: token required");

        std::vector<char> out;
        out.reserve(1u + 1u + 4u + message.token.size());
        out.push_back(static_cast<char>(WireMessageType::LoginResponse));
        out.push_back('\1');
        append_length_prefixed_string(out, message.token);
        return out;
    }

    if (message.error_code == 0u)
        throw std::runtime_error("WireMessageType::encode_login_response_failure: error_code required");

    return std::vector<char>{static_cast<char>(WireMessageType::LoginResponse), '\0',
                             static_cast<char>(message.error_code)};
}


std::vector<char> encode_bind_token_body(const BindToken& message)
{
    if (message.token.empty())
        throw std::runtime_error("WireMessageType::encode_bind_token: token required");

    std::vector<char> out;
    out.reserve(1u + 4u + message.token.size());
    out.push_back(static_cast<char>(WireMessageType::BindToken));
    append_length_prefixed_string(out, message.token);
    return out;
}


std::vector<char> encode_auth_required_body(const AuthRequired&)
{
    return std::vector<char>{static_cast<char>(WireMessageType::AuthRequired)};
}


std::optional<UserChat> decode_user_chat(const std::vector<char>& payload)
{
    if (payload.empty() || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::UserChat)
        return std::nullopt;

    UserChat message;
    message.body.assign(payload.begin() + 1, payload.end());
    return message;
}


std::optional<ServerReceiptAck> decode_server_receipt_ack(const std::vector<char>& payload)
{
    if (payload.size() != 1u || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::ServerReceiptAck)
        return std::nullopt;

    return ServerReceiptAck{};
}


std::optional<HistoryRequest> decode_history_request(const std::vector<char>& payload)
{
    if (payload.empty() || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::HistoryRequest
        || payload.size() != 5u)
        return std::nullopt;

    const auto limit = read_u32_be_at(reinterpret_cast<const unsigned char*>(payload.data() + 1));
    if (limit < 1u)
        return std::nullopt;

    return HistoryRequest{limit};
}


std::optional<HistoryItemPayload> decode_history_item(const std::vector<char>& payload)
{
    if (payload.empty() || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::HistoryItem
        || payload.size() < 14u)
        return std::nullopt;

    const auto* data = reinterpret_cast<const unsigned char*>(payload.data());
    const std::uint64_t message_id = read_u64_be(data + 1);
    const bool is_mine = data[9] != 0u;
    const std::uint32_t body_len = read_u32_be_at(data + 10);

    if (14u + body_len != payload.size())
        return std::nullopt;

    HistoryItemPayload item;
    item.message_id = message_id;
    item.is_mine = is_mine;
    item.body.assign(payload.begin() + 14, payload.end());
    return item;
}


std::optional<HistoryEnd> decode_history_end(const std::vector<char>& payload)
{
    if (payload.size() != 1u || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::HistoryEnd)
        return std::nullopt;

    return HistoryEnd{};
}


std::optional<LoginRequestPayload> decode_login_request(const std::vector<char>& payload)
{
    if (payload.empty() || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::LoginRequest)
        return std::nullopt;

    std::size_t offset = 1u;
    std::string_view login;
    std::string_view password;
    if (!read_length_prefixed_string(login, payload, offset))
        return std::nullopt;
    if (!read_length_prefixed_string(password, payload, offset))
        return std::nullopt;
    if (offset != payload.size())
        return std::nullopt;

    LoginRequestPayload parsed;
    parsed.login.assign(login);
    parsed.password.assign(password);
    return parsed;
}


std::optional<LoginResponsePayload> decode_login_response(const std::vector<char>& payload)
{
    if (payload.empty() || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::LoginResponse
        || payload.size() < 2u)
        return std::nullopt;

    LoginResponsePayload parsed;
    const bool success = static_cast<unsigned char>(payload[1]) != 0u;
    parsed.success = success;

    if (success) {
        std::size_t offset = 2u;
        std::string_view token;
        if (!read_length_prefixed_string(token, payload, offset) || offset != payload.size())
            return std::nullopt;
        parsed.token.assign(token);
        return parsed;
    }

    if (payload.size() != 3u || payload[2] == '\0')
        return std::nullopt;

    parsed.error_code = static_cast<std::uint8_t>(payload[2]);
    return parsed;
}


std::optional<BindToken> decode_bind_token(const std::vector<char>& payload)
{
    if (payload.empty() || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::BindToken)
        return std::nullopt;

    std::size_t offset = 1u;
    std::string_view token;
    if (!read_length_prefixed_string(token, payload, offset) || offset != payload.size())
        return std::nullopt;

    return BindToken{std::string(token)};
}


std::optional<AuthRequired> decode_auth_required(const std::vector<char>& payload)
{
    if (payload.size() != 1u || static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0])) != WireMessageType::AuthRequired)
        return std::nullopt;

    return AuthRequired{};
}


std::string format_user_chat_for_log(const UserChat& message)
{
    std::string out = "UserChat(";
    out += std::to_string(message.body.size());
    out += " bytes): ";

    static constexpr const char* HexDigits = "0123456789abcdef";
    for (const unsigned char c : message.body) {
        if (c == '\n') {
            out += "\\n";
            continue;
        }
        if (c == '\r') {
            out += "\\r";
            continue;
        }
        if (c == '\t') {
            out += "\\t";
            continue;
        }
        if (c == '\\') {
            out += "\\\\";
            continue;
        }
        if (c < 32u) {
            out += "\\x";
            out += HexDigits[c >> 4u];
            out += HexDigits[c & 15u];
            continue;
        }
        out += static_cast<char>(c);
    }
    return out;
}


} // namespace


std::vector<char> encode(const WireMessage& message)
{
    return std::visit(
        [](const auto& typed_message) -> std::vector<char> {
            using T = std::decay_t<decltype(typed_message)>;
            if constexpr (std::is_same_v<T, UserChat>)
                return encode_user_chat_body(typed_message);
            else if constexpr (std::is_same_v<T, ServerReceiptAck>)
                return encode_server_receipt_ack_body(typed_message);
            else if constexpr (std::is_same_v<T, HistoryRequest>)
                return encode_history_request_body(typed_message);
            else if constexpr (std::is_same_v<T, HistoryItemPayload>)
                return encode_history_item_body(typed_message);
            else if constexpr (std::is_same_v<T, HistoryEnd>)
                return encode_history_end_body(typed_message);
            else if constexpr (std::is_same_v<T, LoginRequestPayload>)
                return encode_login_request_body(typed_message);
            else if constexpr (std::is_same_v<T, LoginResponsePayload>)
                return encode_login_response_body(typed_message);
            else if constexpr (std::is_same_v<T, BindToken>)
                return encode_bind_token_body(typed_message);
            else if constexpr (std::is_same_v<T, AuthRequired>)
                return encode_auth_required_body(typed_message);
        },
        message);
}


std::optional<WireMessage> decode(const std::vector<char>& payload)
{
    if (payload.empty())
        return std::nullopt;

    switch (static_cast<WireMessageType>(static_cast<std::uint8_t>(payload[0]))) {
    case WireMessageType::UserChat: {
        const auto message = decode_user_chat(payload);
        return message ? std::optional<WireMessage>{*message} : std::nullopt;
    }
    case WireMessageType::ServerReceiptAck: {
        const auto message = decode_server_receipt_ack(payload);
        return message ? std::optional<WireMessage>{*message} : std::nullopt;
    }
    case WireMessageType::HistoryRequest: {
        const auto message = decode_history_request(payload);
        return message ? std::optional<WireMessage>{*message} : std::nullopt;
    }
    case WireMessageType::HistoryItem: {
        const auto message = decode_history_item(payload);
        return message ? std::optional<WireMessage>{*message} : std::nullopt;
    }
    case WireMessageType::HistoryEnd: {
        const auto message = decode_history_end(payload);
        return message ? std::optional<WireMessage>{*message} : std::nullopt;
    }
    case WireMessageType::LoginRequest: {
        const auto message = decode_login_request(payload);
        return message ? std::optional<WireMessage>{*message} : std::nullopt;
    }
    case WireMessageType::LoginResponse: {
        const auto message = decode_login_response(payload);
        return message ? std::optional<WireMessage>{*message} : std::nullopt;
    }
    case WireMessageType::BindToken: {
        const auto message = decode_bind_token(payload);
        return message ? std::optional<WireMessage>{*message} : std::nullopt;
    }
    case WireMessageType::AuthRequired: {
        const auto message = decode_auth_required(payload);
        return message ? std::optional<WireMessage>{*message} : std::nullopt;
    }
    }

    return std::nullopt;
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


std::string format_for_log(const WireMessage& message)
{
    return std::visit(
        [](const auto& typed_message) -> std::string {
            using T = std::decay_t<decltype(typed_message)>;

            if constexpr (std::is_same_v<T, UserChat>)
                return format_user_chat_for_log(typed_message);
            else if constexpr (std::is_same_v<T, ServerReceiptAck>)
                return "ServerReceiptAck";
            else if constexpr (std::is_same_v<T, HistoryRequest>)
                return "HistoryRequest(" + std::to_string(typed_message.limit) + ')';
            else if constexpr (std::is_same_v<T, HistoryItemPayload>) {
                std::string out = "HistoryItem(id=";
                out += std::to_string(typed_message.message_id);
                out += typed_message.is_mine ? ", mine" : ", peer";
                out += ", ";
                out += std::to_string(typed_message.body.size());
                out += " bytes)";
                return out;
            }
            else if constexpr (std::is_same_v<T, HistoryEnd>)
                return "HistoryEnd";
            else if constexpr (std::is_same_v<T, LoginRequestPayload>)
                return "LoginRequest(login=" + typed_message.login + ')';
            else if constexpr (std::is_same_v<T, LoginResponsePayload>) {
                if (typed_message.success)
                    return "LoginResponse(ok, token_len=" + std::to_string(typed_message.token.size()) + ')';
                return "LoginResponse(error=" + std::to_string(typed_message.error_code) + ')';
            }
            else if constexpr (std::is_same_v<T, BindToken>)
                return "BindToken(len=" + std::to_string(typed_message.token.size()) + ')';
            else if constexpr (std::is_same_v<T, AuthRequired>)
                return "AuthRequired";
        },
        message);
}


std::string format_for_log(const std::vector<char>& payload)
{
    if (payload.empty())
        return "<empty>";

    const auto message = decode(payload);
    if (!message)
        return "<unknown type=" + std::to_string(static_cast<unsigned int>(static_cast<unsigned char>(payload[0])))
               + " len=" + std::to_string(payload.size()) + ">";

    return format_for_log(*message);
}


} // namespace will
