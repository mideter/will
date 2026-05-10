#include "connectionacceptor.h"

#include <netinet/in.h>
#include <sys/socket.h>

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


std::optional<ClientConnection> ConnectionAcceptor::accept_next(const ListenSocketStopSignals& stop_signals)
{
	try {
		return ClientConnection::accept_on_listen(listen_socket_, stop_signals);
	}
	catch (const std::system_error&) {
		if (stop_signals.shutdown_requested())
			return std::nullopt;
		throw;
	}
}


} // namespace will
