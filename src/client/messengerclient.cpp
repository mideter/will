#include "messengerclient.h"

#include <sys/socket.h>

#include <cerrno>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "socketerror.h"


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
}


void MessengerClient::send(std::string_view message) const
{
	if (message.size() > max_payload_bytes)
		throw std::runtime_error("message exceeds max_payload_bytes");

	unsigned char header[4];
	TcpFrame::append_u32_be(header, message.size());
	send_all(socket_.get(), reinterpret_cast<char*>(header), sizeof(header));
	if (!message.empty())
		send_all(socket_.get(), message.data(), message.size());
}


std::optional<std::string> MessengerClient::receiveMessage() const
{
	const int fd = socket_.get();
	unsigned char len_bytes[4];
	if (recv_exact_or_eof_before_first_byte(fd, len_bytes, sizeof(len_bytes)))
		return std::nullopt;

	const std::uint32_t len_u32 = TcpFrame::read_u32_be(len_bytes);
	const std::size_t plen = static_cast<std::size_t>(len_u32);
	if (plen > max_payload_bytes)
		throw std::runtime_error("Will protocol: frame exceeds max_payload_bytes");

	std::string body(plen, '\0');
	if (plen > 0)
		recv_exact(fd, reinterpret_cast<unsigned char*>(body.data()), plen);
	return body;
}


void MessengerClient::shutdown() const
{
	::shutdown(socket_.get(), SHUT_RDWR);
}


} // namespace will
