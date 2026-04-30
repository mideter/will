#include "messengerclient.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#include <vector>

#include "socketerror.h"


MessengerClient::MessengerClient()
	: socket_(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
{}


void MessengerClient::connect(ServerAddress server)
{
	if (::connect(socket_.get(), reinterpret_cast<sockaddr*>(&server.address_), sizeof(server.address_)) < 0)
		throw SocketError("connect failed");
}


void MessengerClient::send(std::string_view message) const
{
	std::size_t total_sent = 0;
	while (total_sent < message.size()) {
		const ssize_t sent = ::send(
			socket_.get(),
			message.data() + total_sent,
			message.size() - total_sent,
			MSG_NOSIGNAL
		);

		if (sent < 0)
			throw SocketError("send failed");

		if (sent == 0)
			throw SocketError(EPIPE, "send failed");

		total_sent += static_cast<std::size_t>(sent);
	}
}


std::string MessengerClient::receive(std::size_t max_bytes) const
{
	if (max_bytes == 0)
		return {};

	std::vector<char> buffer(max_bytes + 1, '\0');
	const ssize_t bytes_received = ::recv(socket_.get(), buffer.data(), max_bytes, 0);
	
	if (bytes_received < 0)
		throw SocketError("recv failed");

	return std::string(buffer.data(), static_cast<std::size_t>(bytes_received));
}


void MessengerClient::shutdown() const
{
	::shutdown(socket_.get(), SHUT_RDWR);
}
