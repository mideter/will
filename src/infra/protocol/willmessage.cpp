#include "willmessage.h"

#include <cstring>
#include <stdexcept>
#include <string>


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
    if (value.size() > WillMessage::MaxAuthFieldBytes)
        throw std::runtime_error("WillMessage: auth field exceeds MaxAuthFieldBytes");

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

    if (len == 0u || len > WillMessage::MaxAuthFieldBytes || offset + len > payload.size())
        return false;

    field = std::string_view(payload.data() + offset, len);
    offset += len;
    return true;
}


} // namespace


std::vector<char> WillMessage::encode_user_chat(std::string_view utf8_body)
{
    const std::size_t total = 1u + utf8_body.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WillMessage::encode_user_chat: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out(total);
    out[0] = static_cast<char>(UserChat);
    if (!utf8_body.empty())
        std::memcpy(out.data() + 1, utf8_body.data(), utf8_body.size());
    return out;
}


std::vector<char> WillMessage::encode_server_receipt_ack()
{
    return std::vector<char>{static_cast<char>(ServerReceiptAck)};
}


std::vector<char> WillMessage::encode_history_request(const std::uint32_t limit)
{
    if (limit < 1u)
        throw std::runtime_error("WillMessage::encode_history_request: limit must be at least 1");

    std::vector<char> out;
    out.reserve(5);
    out.push_back(static_cast<char>(HistoryRequest));
    append_u32_be(out, limit);
    return out;
}


std::vector<char> WillMessage::encode_history_item(const std::uint64_t message_id, const bool is_mine,
                                                   const std::string_view utf8_body)
{
    const std::size_t total = 1u + 8u + 1u + 4u + utf8_body.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WillMessage::encode_history_item: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out;
    out.reserve(total);
    out.push_back(static_cast<char>(HistoryItem));
    append_u64_be(out, message_id);
    out.push_back(is_mine ? '\1' : '\0');
    append_u32_be(out, static_cast<std::uint32_t>(utf8_body.size()));
    if (!utf8_body.empty())
        out.insert(out.end(), utf8_body.begin(), utf8_body.end());
    return out;
}


std::vector<char> WillMessage::encode_history_end()
{
    return std::vector<char>{static_cast<char>(HistoryEnd)};
}


std::vector<char> WillMessage::encode_login_request(const std::string_view login,
                                                    const std::string_view password)
{
    if (login.empty() || password.empty())
        throw std::runtime_error("WillMessage::encode_login_request: login and password required");

    std::vector<char> out;
    out.reserve(1u + 8u + login.size() + password.size());
    out.push_back(static_cast<char>(LoginRequest));
    append_length_prefixed_string(out, login);
    append_length_prefixed_string(out, password);

    if (out.size() > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("WillMessage::encode_login_request: payload exceeds TcpFrame::MaxPayloadBytes");

    return out;
}


std::vector<char> WillMessage::encode_login_response_success(const std::string_view token)
{
    if (token.empty())
        throw std::runtime_error("WillMessage::encode_login_response_success: token required");

    std::vector<char> out;
    out.reserve(1u + 1u + 4u + token.size());
    out.push_back(static_cast<char>(LoginResponse));
    out.push_back('\1');
    append_length_prefixed_string(out, token);
    return out;
}


std::vector<char> WillMessage::encode_login_response_failure(const std::uint8_t error_code)
{
    if (error_code == 0u)
        throw std::runtime_error("WillMessage::encode_login_response_failure: error_code required");

    return std::vector<char>{static_cast<char>(LoginResponse), '\0', static_cast<char>(error_code)};
}


std::vector<char> WillMessage::encode_bind_token(const std::string_view token)
{
    if (token.empty())
        throw std::runtime_error("WillMessage::encode_bind_token: token required");

    std::vector<char> out;
    out.reserve(1u + 4u + token.size());
    out.push_back(static_cast<char>(BindToken));
    append_length_prefixed_string(out, token);
    return out;
}


std::vector<char> WillMessage::encode_auth_required()
{
    return std::vector<char>{static_cast<char>(AuthRequired)};
}


bool WillMessage::is_valid_client_to_server_payload(const std::vector<char>& payload) noexcept
{
    if (payload.empty())
        return false;
    const auto t = static_cast<std::uint8_t>(payload[0]);
    if (t == UserChat)
        return true;
    if (t == HistoryRequest)
        return payload.size() == 5u && parse_history_request_limit(payload).has_value();
    if (t == LoginRequest)
        return parse_login_request(payload).has_value();
    if (t == BindToken)
        return parse_bind_token(payload).has_value();
    return false;
}


bool WillMessage::is_user_chat(const std::vector<char>& payload) noexcept
{
    return !payload.empty() && static_cast<std::uint8_t>(payload[0]) == UserChat;
}


bool WillMessage::is_server_receipt_ack(const std::vector<char>& payload) noexcept
{
    return payload.size() == 1u && static_cast<std::uint8_t>(payload[0]) == ServerReceiptAck;
}


bool WillMessage::is_history_request(const std::vector<char>& payload) noexcept
{
    return !payload.empty() && static_cast<std::uint8_t>(payload[0]) == HistoryRequest;
}


bool WillMessage::is_history_item(const std::vector<char>& payload) noexcept
{
    return !payload.empty() && static_cast<std::uint8_t>(payload[0]) == HistoryItem;
}


bool WillMessage::is_history_end(const std::vector<char>& payload) noexcept
{
    return payload.size() == 1u && static_cast<std::uint8_t>(payload[0]) == HistoryEnd;
}


bool WillMessage::is_login_request(const std::vector<char>& payload) noexcept
{
    return !payload.empty() && static_cast<std::uint8_t>(payload[0]) == LoginRequest;
}


bool WillMessage::is_login_response(const std::vector<char>& payload) noexcept
{
    return !payload.empty() && static_cast<std::uint8_t>(payload[0]) == LoginResponse;
}


bool WillMessage::is_bind_token(const std::vector<char>& payload) noexcept
{
    return !payload.empty() && static_cast<std::uint8_t>(payload[0]) == BindToken;
}


bool WillMessage::is_auth_required(const std::vector<char>& payload) noexcept
{
    return payload.size() == 1u && static_cast<std::uint8_t>(payload[0]) == AuthRequired;
}


std::optional<std::uint32_t> WillMessage::parse_history_request_limit(const std::vector<char>& payload)
{
    if (!is_history_request(payload) || payload.size() != 5u)
        return std::nullopt;

    const auto limit = read_u32_be_at(reinterpret_cast<const unsigned char*>(payload.data() + 1));
    if (limit < 1u)
        return std::nullopt;

    return limit;
}


std::optional<HistoryItemPayload> WillMessage::parse_history_item(const std::vector<char>& payload)
{
    if (!is_history_item(payload) || payload.size() < 14u)
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


std::optional<LoginRequestPayload> WillMessage::parse_login_request(const std::vector<char>& payload)
{
    if (!is_login_request(payload))
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


std::optional<LoginResponsePayload> WillMessage::parse_login_response(const std::vector<char>& payload)
{
    if (!is_login_response(payload) || payload.size() < 2u)
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


std::optional<std::string> WillMessage::parse_bind_token(const std::vector<char>& payload)
{
    if (!is_bind_token(payload))
        return std::nullopt;

    std::size_t offset = 1u;
    std::string_view token;
    if (!read_length_prefixed_string(token, payload, offset) || offset != payload.size())
        return std::nullopt;

    return std::string(token);
}


std::string WillMessage::format_payload_for_log(const std::vector<char>& payload)
{
    if (payload.empty())
        return "<empty>";

    if (is_server_receipt_ack(payload))
        return "ServerReceiptAck";

    if (is_auth_required(payload))
        return "AuthRequired";

    if (is_history_end(payload))
        return "HistoryEnd";

    if (is_login_response(payload)) {
        const auto response = parse_login_response(payload);
        if (!response)
            return "LoginResponse(invalid)";
        if (response->success)
            return "LoginResponse(ok, token_len=" + std::to_string(response->token.size()) + ')';
        return "LoginResponse(error=" + std::to_string(response->error_code) + ')';
    }

    if (is_login_request(payload)) {
        const auto request = parse_login_request(payload);
        if (!request)
            return "LoginRequest(invalid)";
        return "LoginRequest(login=" + request->login + ')';
    }

    if (is_bind_token(payload)) {
        const auto token = parse_bind_token(payload);
        return token ? "BindToken(len=" + std::to_string(token->size()) + ')' : "BindToken(invalid)";
    }

    if (is_history_request(payload)) {
        const auto limit = parse_history_request_limit(payload);
        std::string out = "HistoryRequest(";
        out += limit ? std::to_string(*limit) : "?";
        out += ')';
        return out;
    }

    if (is_history_item(payload)) {
        const auto item = parse_history_item(payload);
        std::string out = "HistoryItem(";
        if (item) {
            out += "id=";
            out += std::to_string(item->message_id);
            out += item->is_mine ? ", mine" : ", peer";
            out += ", ";
            out += std::to_string(item->body.size());
            out += " bytes";
        }
        else {
            out += "invalid";
        }
        out += ')';
        return out;
    }

    if (is_user_chat(payload)) {
        std::string out = "UserChat(";
        out += std::to_string(payload.size() > 0 ? payload.size() - 1u : 0u);
        out += " bytes): ";

        static constexpr const char* HexDigits = "0123456789abcdef";
        for (std::size_t i = 1; i < payload.size(); ++i) {
            const unsigned char c = static_cast<unsigned char>(payload[i]);
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

    std::string out = "<unknown type=";
    out += std::to_string(static_cast<unsigned int>(static_cast<unsigned char>(payload[0])));
    out += " len=";
    out += std::to_string(payload.size());
    out += ">";
    return out;
}


} // namespace will
