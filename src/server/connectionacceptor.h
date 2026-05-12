#pragma once

#include <optional>

#include "clientconnection.h"
#include "listensocketstopsignals.h"
#include "sockethandle.h"


namespace will {


class ConnectionAcceptor {
public:
	ConnectionAcceptor();

	std::optional<ClientConnection> accept_next();

private:
	ClientConnection accept_incoming_connection();

	SocketHandle listen_socket_;
	ListenSocketStopSignals stop_signals_;

	static constexpr int Backlog = 5;
};


} // namespace will
