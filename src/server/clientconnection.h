#pragma once

#include <cstddef>

#include "clientaddress.h"
#include "sockethandle.h"


namespace will {


class ClientConnection {
public:
	ClientConnection(SocketHandle socket, ClientAddress address);

	int socket_fd() const noexcept;

	const ClientAddress& address() const noexcept;
	
	bool recv_some(char* buffer, std::size_t len, std::size_t& received) const;
	void send_all(const char* data, std::size_t len) const;
	void shutdown() const;

private:
	SocketHandle socket_;
	ClientAddress address_;
};


} // namespace will
