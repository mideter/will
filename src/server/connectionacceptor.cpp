#include "connectionacceptor.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <iostream>
#include <system_error>

#include "defaultwillserver.h"
#include "listensocketstopsignals.h"
#include "serveraddress.h"
#include "socketerror.h"


namespace will {


ConnectionAcceptor::ConnectionAcceptor()
	: listen_socket_(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
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


int ConnectionAcceptor::listen_fd() const noexcept
{
	return listen_socket_.get();
}


std::optional<AcceptedConnection> ConnectionAcceptor::accept_next(const ListenSocketStopSignals& stop_signals)
{
	try {
		ClientConnection connection = ClientConnection::accept_on_listen(listen_socket_, stop_signals);
		const int sig_slot = ListenSocketStopSignals::register_chat_peer_fd(connection.socket_fd());
		
		if (sig_slot < 0)
			std::cerr << "Warning: chat peer FD registry full; graceful signal stop may omit this peer\n";

		return AcceptedConnection{std::move(connection), sig_slot};
	}
	catch (const std::system_error&) {
		if (stop_signals.shutdown_requested())
			return std::nullopt;
		throw;
	}
}


} // namespace will
