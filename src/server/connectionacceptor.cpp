#include "connectionacceptor.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cerrno>
#include <system_error>

#include "defaultwillserver.h"
#include "serveraddress.h"
#include "socketerror.h"


namespace will {


ConnectionAcceptor::ConnectionAcceptor()
	: listen_socket_(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
	, stop_signals_{listen_socket_.get()}
{
	int opt = 1;
	if (::setsockopt(listen_socket_.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw SocketError("setsockopt failed");

	ServerAddress server = ServerAddress::any(defaultWillServerAddress().port_);

	if (::bind(listen_socket_.get(), reinterpret_cast<sockaddr*>(&server.address_), sizeof(server.address_)) < 0)
		throw SocketError("bind failed");

	if (::listen(listen_socket_.get(), Backlog) < 0)
		throw SocketError("listen failed");
}


ClientConnection ConnectionAcceptor::accept_incoming_connection()
{
	sockaddr_in peer{};
	socklen_t peer_len = sizeof(peer);

	while (true) {
		const int fd = ::accept(listen_socket_.get(),
								reinterpret_cast<sockaddr*>(&peer),
								&peer_len);
		if (fd >= 0)
			return ClientConnection(SocketHandle{fd}, ClientAddress{peer});

		if (errno == EINTR) {
			if (stop_signals_.shutdown_requested())
				throw std::system_error(EINTR, std::generic_category(), "accept interrupted during shutdown");
			continue;
		}

		throw std::system_error(errno, std::generic_category(), "accept failed");
	}
}


std::optional<ClientConnection> ConnectionAcceptor::accept_next()
try {
	return accept_incoming_connection();
}
catch (const std::system_error&) {
	if (stop_signals_.shutdown_requested())
		return std::nullopt;
	
	throw;
}


} // namespace will
