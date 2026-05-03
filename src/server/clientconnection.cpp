#include "clientconnection.h"

#include <sys/socket.h>

#include <cerrno>
#include <stdexcept>
#include <utility>

#include "socketerror.h"


ClientConnection::ClientConnection(SocketHandle socket, ClientAddress address)
	: socket_(std::move(socket))
	, address_(std::move(address))
{}


ClientConnection ClientConnection::accept_from(const SocketHandle& server_socket)
{
	sockaddr_in peer{};
	socklen_t peer_len = sizeof(peer);
	SocketHandle socket(::accept(server_socket.get(),
								 reinterpret_cast<sockaddr*>(&peer),
								 &peer_len));

	return ClientConnection(std::move(socket), ClientAddress{peer});
}


const ClientAddress& ClientConnection::address() const noexcept
{
	return address_;
}


bool ClientConnection::recv_some(char* buffer, std::size_t len, std::size_t& received) const
{
	ssize_t n = 0;
	while (true) {
		n = ::recv(socket_.get(), buffer, len, 0);
		if (n >= 0)
			break;
		if (errno != EINTR)
			throw SocketError("recv failed");
	}

	if (n == 0) {
		received = 0;
		return false;
	}

	received = static_cast<std::size_t>(n);
	return true;
}


void ClientConnection::send_all(const char* data, std::size_t len) const
{
	std::size_t sent = 0;
	while (sent < len) {
		const ssize_t n = ::send(socket_.get(), data + sent, len - sent, MSG_NOSIGNAL);

		if (n < 0) {
			if (errno == EINTR)
				continue;
			throw SocketError("send failed");
		}

		if (n == 0)
			throw std::runtime_error("send failed: connection closed");

		sent += static_cast<std::size_t>(n);
	}
}


void ClientConnection::shutdown() const
{
	::shutdown(socket_.get(), SHUT_RDWR);
}
