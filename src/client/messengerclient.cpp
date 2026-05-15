#include "messengerclient.h"

#include <sys/socket.h>

#include <cerrno>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "socketerror.h"
#include "willmessage.h"


namespace will {


namespace {

void send_all(int sock, const char* data, std::size_t len)
{
	std::size_t sent = 0;
	while (sent < len) {
		const ssize_t n =
			::send(sock, data + sent, len - sent, MSG_NOSIGNAL);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			throw SocketError("send failed");
		}
		if (n == 0)
			throw SocketError(EPIPE, "send failed");
		sent += static_cast<std::size_t>(n);
	}
}


/** Exactly {@code len} bytes, or EOF before/on first byte ({@code true}), or protocol error after partial read. */
bool recv_exact_or_eof_before_first_byte(int sock, unsigned char* data, std::size_t len)
{
	std::size_t got = 0;
	while (got < len) {
		const ssize_t n = ::recv(sock, data + got, len - got, 0);
		if (n > 0) {
			got += static_cast<std::size_t>(n);
			continue;
		}
		if (n == 0) {
			if (got == 0)
				return true;
			throw std::runtime_error("Will protocol: connection closed mid-frame");
		}
		if (errno != EINTR)
			throw SocketError("recv failed");
	}
	return false;
}


void recv_exact(int sock, unsigned char* data, std::size_t len)
{
	if (recv_exact_or_eof_before_first_byte(sock, data, len))
		throw std::runtime_error("Will protocol: unexpected end of stream mid-frame");
}


} // namespace


MessengerClient::MessengerClient()
	: socket_(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
{}


void MessengerClient::connect(ServerAddress server)
{
	if (::connect(socket_.get(), reinterpret_cast<sockaddr*>(&server.address_),
				  sizeof(server.address_)) < 0)
		throw SocketError("connect failed");

	socket_.enable_tcp_keepalive();
}


void MessengerClient::send(std::string_view utf8_chat_body) const
{
	const std::vector<char> payload = WillMessage::encode_user_chat(utf8_chat_body);
	if (payload.size() > MaxPayloadBytes)
		throw std::runtime_error("message exceeds MaxPayloadBytes");

	unsigned char header[4];
	TcpFrame::append_u32_be(header, payload.size());
	send_all(socket_.get(), reinterpret_cast<char*>(header), sizeof(header));
	send_all(socket_.get(), payload.data(), payload.size());
}


std::optional<InboundMessage> MessengerClient::receiveMessage() const
{
	const int fd = socket_.get();
	unsigned char len_bytes[4];
	if (recv_exact_or_eof_before_first_byte(fd, len_bytes, sizeof(len_bytes)))
		return std::nullopt;

	const std::uint32_t len_u32 = TcpFrame::read_u32_be(len_bytes);
	const std::size_t plen = static_cast<std::size_t>(len_u32);
	
	if (plen > MaxPayloadBytes)
		throw std::runtime_error("Will protocol: frame exceeds MaxPayloadBytes");
	if (plen == 0)
		throw std::runtime_error("Will protocol: empty typed payload is invalid");

	std::vector<char> payload(plen);
	recv_exact(fd, reinterpret_cast<unsigned char*>(payload.data()), plen);

	if (WillMessage::is_server_receipt_ack(payload))
		return InboundMessage{std::in_place_type<ServerReceiptAck>};

	if (WillMessage::is_user_chat(payload))
		return InboundMessage{std::in_place_type<std::string>, std::string(payload.begin() + 1, payload.end())};

	throw std::runtime_error("Will protocol: unknown message type");
}


void MessengerClient::shutdown() const
{
	::shutdown(socket_.get(), SHUT_RDWR);
}


} // namespace will
