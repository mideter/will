#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "willprotocol.h"


namespace will {


enum class WireMessageType : std::uint8_t {
    UserChat = 1,
    ServerReceiptAck = 2,
    HistoryRequest = 3,
    HistoryItem = 4,
    HistoryEnd = 5,
    LoginRequest = 6,
    LoginResponse = 7,
    BindToken = 8,
    AuthRequired = 9,
};


enum class LoginError : std::uint8_t {
    InvalidCredentials = 1,
    ExpiredToken = 2,
};


struct UserChat {
    std::string body;

    bool operator==(const UserChat& other) const noexcept { return body == other.body; }
};


struct ServerReceiptAck {
    bool operator==(const ServerReceiptAck&) const noexcept { return true; }
};


struct HistoryRequest {
    std::uint32_t limit = 0;

    bool operator==(const HistoryRequest& other) const noexcept { return limit == other.limit; }
};


struct HistoryItemPayload {
    std::uint64_t message_id = 0;
    bool is_mine = false;
    std::string body;

    bool operator==(const HistoryItemPayload& other) const noexcept
    {
        return message_id == other.message_id && is_mine == other.is_mine && body == other.body;
    }
};


struct HistoryEnd {
    bool operator==(const HistoryEnd&) const noexcept { return true; }
};


struct LoginRequestPayload {
    std::string login;
    std::string password;

    bool operator==(const LoginRequestPayload& other) const noexcept
    {
        return login == other.login && password == other.password;
    }
};


struct LoginResponsePayload {
    bool success = false;
    std::string token;
    std::uint8_t error_code = 0;

    bool operator==(const LoginResponsePayload& other) const noexcept
    {
        return success == other.success && token == other.token && error_code == other.error_code;
    }
};


struct BindToken {
    std::string token;

    bool operator==(const BindToken& other) const noexcept { return token == other.token; }
};


struct AuthRequired {
    bool operator==(const AuthRequired&) const noexcept { return true; }
};


using WireMessageEntity = std::variant<
    UserChat,
    ServerReceiptAck,
    HistoryRequest,
    HistoryItemPayload,
    HistoryEnd,
    LoginRequestPayload,
    LoginResponsePayload,
    BindToken,
    AuthRequired>;


std::vector<char> encode(const WireMessageEntity& message);
std::optional<WireMessageEntity> decode(const std::vector<char>& payload);

bool is_client_to_server(const WireMessageEntity& message) noexcept;

std::string format_for_log(const WireMessageEntity& message);
std::string format_for_log(const std::vector<char>& payload);


/** Typed payload inside {@link TcpFrame}: first byte {@code MessageType}, then type-specific body. */
class WireMessage {
public:
    static constexpr std::uint8_t UserChat = static_cast<std::uint8_t>(WireMessageType::UserChat);
    static constexpr std::uint8_t ServerReceiptAck = static_cast<std::uint8_t>(WireMessageType::ServerReceiptAck);
    static constexpr std::uint8_t HistoryRequest = static_cast<std::uint8_t>(WireMessageType::HistoryRequest);
    static constexpr std::uint8_t HistoryItem = static_cast<std::uint8_t>(WireMessageType::HistoryItem);
    static constexpr std::uint8_t HistoryEnd = static_cast<std::uint8_t>(WireMessageType::HistoryEnd);
    static constexpr std::uint8_t LoginRequest = static_cast<std::uint8_t>(WireMessageType::LoginRequest);
    static constexpr std::uint8_t LoginResponse = static_cast<std::uint8_t>(WireMessageType::LoginResponse);
    static constexpr std::uint8_t BindToken = static_cast<std::uint8_t>(WireMessageType::BindToken);
    static constexpr std::uint8_t AuthRequired = static_cast<std::uint8_t>(WireMessageType::AuthRequired);

    static constexpr std::uint8_t LoginErrorInvalidCredentials =
        static_cast<std::uint8_t>(LoginError::InvalidCredentials);
    static constexpr std::uint8_t LoginErrorExpiredToken = static_cast<std::uint8_t>(LoginError::ExpiredToken);

    static constexpr std::uint32_t MaxHistoryRequestLimit = 1000;
    static constexpr std::uint32_t MaxAuthFieldBytes = 4096;

    static std::vector<char> encode_user_chat(std::string_view utf8_body);
    static std::vector<char> encode_server_receipt_ack();
    static std::vector<char> encode_history_request(std::uint32_t limit);
    static std::vector<char> encode_history_item(std::uint64_t message_id, bool is_mine,
                                                 std::string_view utf8_body);
    static std::vector<char> encode_history_end();
    static std::vector<char> encode_login_request(std::string_view login, std::string_view password);
    static std::vector<char> encode_login_response_success(std::string_view token);
    static std::vector<char> encode_login_response_failure(std::uint8_t error_code);
    static std::vector<char> encode_bind_token(std::string_view token);
    static std::vector<char> encode_auth_required();

    /** Structurally valid client → server types (auth gating is enforced in the adapter). */
    static bool is_valid_client_to_server_payload(const std::vector<char>& payload) noexcept;

    static bool is_user_chat(const std::vector<char>& payload) noexcept;
    static bool is_server_receipt_ack(const std::vector<char>& payload) noexcept;
    static bool is_history_request(const std::vector<char>& payload) noexcept;
    static bool is_history_item(const std::vector<char>& payload) noexcept;
    static bool is_history_end(const std::vector<char>& payload) noexcept;
    static bool is_login_request(const std::vector<char>& payload) noexcept;
    static bool is_login_response(const std::vector<char>& payload) noexcept;
    static bool is_bind_token(const std::vector<char>& payload) noexcept;
    static bool is_auth_required(const std::vector<char>& payload) noexcept;

    static std::optional<std::uint32_t> parse_history_request_limit(const std::vector<char>& payload);
    static std::optional<HistoryItemPayload> parse_history_item(const std::vector<char>& payload);
    static std::optional<LoginRequestPayload> parse_login_request(const std::vector<char>& payload);
    static std::optional<LoginResponsePayload> parse_login_response(const std::vector<char>& payload);
    static std::optional<std::string> parse_bind_token(const std::vector<char>& payload);

    /** Single-line UTF-8 safe for journald/terminals: never dumps raw framing bytes. */
    static std::string format_payload_for_log(const std::vector<char>& payload);

    WireMessage() = delete;
};


} // namespace will
