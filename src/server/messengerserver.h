#pragma once

#include "clientconnection.h"


namespace will {


class SocketHandle;
class ListenSocketStopSignals;


class MessengerServer {
public:
	void run() const;

private:
	SocketHandle 	create_listen_socket() const;
	void 			bind_and_listen(const SocketHandle& server_socket) const;
	void 			serve_clients(const SocketHandle& server_socket, const ListenSocketStopSignals& stop_signals) const;
	
	static void run_chat_session(const ClientConnection& first, const ClientConnection& second);
	static void relay_messages(const ClientConnection& from, const ClientConnection& to);

	static constexpr int Backlog = 5;
};


} // namespace will
