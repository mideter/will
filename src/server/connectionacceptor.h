#pragma once

#include <optional>

#include "clientconnection.h"
#include "sockethandle.h"


namespace will {


class ListenSocketStopSignals;


class ConnectionAcceptor {
public:
	ConnectionAcceptor();

	int listen_fd() const noexcept;

	std::optional<ClientConnection> accept_next(const ListenSocketStopSignals& stop_signals);

private:
	SocketHandle listen_socket_;

	static constexpr int Backlog = 5;
};


} // namespace will
