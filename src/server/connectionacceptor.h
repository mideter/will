#pragma once

#include <optional>

#include "clientconnection.h"
#include "sockethandle.h"


namespace will {


class ListenSocketStopSignals;


/** Result of accept_next: new peer and ListenSocketStopSignals registry slot (or -1 if full). */
struct AcceptedConnection {
	ClientConnection connection;
	int sig_slot = -1;
};


class ConnectionAcceptor {
public:
	ConnectionAcceptor();

	int listen_fd() const noexcept;

	std::optional<AcceptedConnection> accept_next(const ListenSocketStopSignals& stop_signals);

private:
	SocketHandle listen_socket_;

	static constexpr int Backlog = 5;
};


} // namespace will
