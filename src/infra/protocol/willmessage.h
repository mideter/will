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


/** Typed payload inside {@link TcpFrame}: first byte {@code MessageType}, then type-specific body. */
class WillMessage {
public:
    static constexpr std::uint8_t UserChat = 1;
    static constexpr std::uint8_t ServerReceiptAck = 2;
    static constexpr std::uint8_t HistoryRequest = 3;
    static constexpr std::uint8_t HistoryItem = 4;
    static constexpr std::uint8_t HistoryEnd = 5;

    static constexpr std::uint32_t MaxHistoryRequestLimit = 1000;

    static std::vector<char> encode_user_chat(std::string_view utf8_body);
    static std::vector<char> encode_server_receipt_ack();
    static std::vector<char> encode_history_request(std::uint32_t limit);
    static std::vector<char> encode_history_item(std::uint64_t message_id, bool is_mine,
                                                 std::string_view utf8_body);
    static std::vector<char> encode_history_end();

    /** Non-empty and type {@code UserChat} or {@code HistoryRequest} (client may only send these). */
    static bool is_valid_client_to_server_payload(const std::vector<char>& payload) noexcept;

    static bool is_user_chat(const std::vector<char>& payload) noexcept;
    static bool is_server_receipt_ack(const std::vector<char>& payload) noexcept;
    static bool is_history_request(const std::vector<char>& payload) noexcept;
    static bool is_history_item(const std::vector<char>& payload) noexcept;
    static bool is_history_end(const std::vector<char>& payload) noexcept;

    static std::optional<std::uint32_t> parse_history_request_limit(const std::vector<char>& payload);
    static std::optional<HistoryItemPayload> parse_history_item(const std::vector<char>& payload);

    /** Single-line UTF-8 safe for journald/terminals: never dumps raw framing bytes. */
    static std::string format_payload_for_log(const std::vector<char>& payload);

    WillMessage() = delete;
};


} // namespace will
