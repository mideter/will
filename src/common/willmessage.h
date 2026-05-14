#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "willprotocol.h"


namespace will {


/** Typed payload inside {@link TcpFrame}: first byte {@code MessageType}, then type-specific body. */
class WillMessage {
public:
	static constexpr std::uint8_t kUserChat = 1;
	static constexpr std::uint8_t kServerReceiptAck = 2;

	static std::vector<char> encode_user_chat(std::string_view utf8_body);
	static std::vector<char> encode_server_receipt_ack();

	/** Non-empty and type {@code kUserChat} (client may only send this). */
	static bool is_valid_client_to_server_payload(const std::vector<char>& payload) noexcept;

	static bool is_user_chat(const std::vector<char>& payload) noexcept;
	static bool is_server_receipt_ack(const std::vector<char>& payload) noexcept;

	/** Single-line UTF-8 safe for journald/terminals: never dumps raw framing bytes. */
	static std::string format_payload_for_log(const std::vector<char>& payload);

	WillMessage() = delete;
};


} // namespace will
