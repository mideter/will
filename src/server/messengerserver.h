#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include "clientconnection.h"
#include "port.h"


class SocketHandle;
class ListenSocketStopSignals;


class MessengerServer {
public:
	explicit MessengerServer();

	void run() const;

private:
	struct AuthLineReadResult {
		std::optional<std::string> line;
		bool too_long = false;
	};

	SocketHandle 	create_listen_socket() const;
	void 			bind_and_listen(const SocketHandle& server_socket) const;
	void 			serve_clients(const SocketHandle& server_socket, const ListenSocketStopSignals& stop_signals) const;
	
	static void log_client_connected(const ClientConnection& client_connection);
	static AuthLineReadResult receive_line(const ClientConnection& client);
	static bool authenticate_client(const ClientConnection& client);
	static void run_chat_session(const ClientConnection& first, const ClientConnection& second);
	static void relay_messages(const ClientConnection& from, const ClientConnection& to);

	static constexpr std::size_t BufferSize = 1024;
	static constexpr std::size_t MaxAuthLineSize = 128;
	static constexpr int Backlog = 5;

	Port port_;
};
