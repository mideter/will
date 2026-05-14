#include "willmessage.h"

#include <cstring>
#include <stdexcept>
#include <string>


namespace will {


std::vector<char> WillMessage::encode_user_chat(std::string_view utf8_body)
{
	const std::size_t total = 1u + utf8_body.size();
	if (total > TcpFrame::max_payload_bytes)
		throw std::runtime_error("WillMessage::encode_user_chat: payload exceeds TcpFrame::max_payload_bytes");

	std::vector<char> out(total);
	out[0] = static_cast<char>(kUserChat);
	if (!utf8_body.empty())
		std::memcpy(out.data() + 1, utf8_body.data(), utf8_body.size());
	return out;
}


std::vector<char> WillMessage::encode_server_receipt_ack()
{
	return std::vector<char>{static_cast<char>(kServerReceiptAck)};
}


bool WillMessage::is_valid_client_to_server_payload(const std::vector<char>& payload) noexcept
{
	if (payload.empty())
		return false;
	const auto t = static_cast<std::uint8_t>(payload[0]);
	if (t == kUserChat)
		return true;
	return false;
}


bool WillMessage::is_user_chat(const std::vector<char>& payload) noexcept
{
	return !payload.empty() && static_cast<std::uint8_t>(payload[0]) == kUserChat;
}


bool WillMessage::is_server_receipt_ack(const std::vector<char>& payload) noexcept
{
	return payload.size() == 1u && static_cast<std::uint8_t>(payload[0]) == kServerReceiptAck;
}


std::string WillMessage::format_payload_for_log(const std::vector<char>& payload)
{
	if (payload.empty())
		return "<empty>";

	if (is_server_receipt_ack(payload))
		return "ServerReceiptAck";

	if (is_user_chat(payload)) {
		std::string out = "UserChat(";
		out += std::to_string(payload.size() > 0 ? payload.size() - 1u : 0u);
		out += " bytes): ";

		static constexpr const char* hex = "0123456789abcdef";
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
				out += hex[c >> 4u];
				out += hex[c & 15u];
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
