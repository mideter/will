#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "willprotocol.h"


namespace will {


struct HistoryItemPayload {
    std::uint64_t message_id = 0;
    bool is_mine = false;
    std::string body;
};


struct LoginRequestPayload {
    std::string login;
    std::string password;
};


struct LoginResponsePayload {
    bool success = false;
    std::string token;
    std::uint8_t error_code = 0;
};


/** Typed payload inside {@link TcpFrame}: first byte {@code MessageType}, then type-specific body. */
class WireMessage {
public:
    static constexpr std::uint8_t UserChat = 1;
    static constexpr std::uint8_t ServerReceiptAck = 2;
    static constexpr std::uint8_t HistoryRequest = 3;
    static constexpr std::uint8_t HistoryItem = 4;
    static constexpr std::uint8_t HistoryEnd = 5;
    static constexpr std::uint8_t LoginRequest = 6;
    static constexpr std::uint8_t LoginResponse = 7;
    static constexpr std::uint8_t BindToken = 8;
    static constexpr std::uint8_t AuthRequired = 9;

    static constexpr std::uint8_t LoginErrorInvalidCredentials = 1;
    static constexpr std::uint8_t LoginErrorExpiredToken = 2;

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
