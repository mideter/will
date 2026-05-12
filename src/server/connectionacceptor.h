#pragma once

#include <optional>

#include "clientconnection.h"
#include "listensocketstopsignals.h"
#include "sockethandle.h"


namespace will {


/** Result of accept_next: new peer and ListenSocketStopSignals registry slot (or -1 if full). */
struct AcceptedConnection {
	ClientConnection connection;
	int sig_slot = -1;
};


class ConnectionAcceptor {
public:
	ConnectionAcceptor();

	std::optional<AcceptedConnection> accept_next();

private:
	SocketHandle listen_socket_;
	ListenSocketStopSignals stop_signals_;

	static constexpr int Backlog = 5;
};


} // namespace will
