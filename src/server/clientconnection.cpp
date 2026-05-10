#include "clientconnection.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "listensocketstopsignals.h"
#include "socketerror.h"


namespace will {


ClientConnection::ClientConnection(SocketHandle socket, ClientAddress address)
	: socket_(std::move(socket))
	, address_(std::move(address))
{}


ClientConnection ClientConnection::accept_on_listen(const SocketHandle& listen_socket,
													const ListenSocketStopSignals& stop_signals)
{
	sockaddr_in peer{};
	socklen_t peer_len = sizeof(peer);

	while (true) {
		const int fd = ::accept(listen_socket.get(),
								reinterpret_cast<sockaddr*>(&peer),
								&peer_len);
		if (fd >= 0)
			return ClientConnection(SocketHandle{fd}, ClientAddress{peer});

		if (errno == EINTR) {
			if (stop_signals.shutdown_requested())
				throw std::system_error(EINTR, std::generic_category(), "accept interrupted during shutdown");
			continue;
		}

		throw std::system_error(errno, std::generic_category(), "accept failed");
	}
}


int ClientConnection::socket_fd() const noexcept
{
	return socket_.get();
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


} // namespace will
