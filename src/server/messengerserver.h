#pragma once

#include <cstddef>

#include "clientconnection.h"


class SocketHandle;
class ListenSocketStopSignals;


class MessengerServer {
public:
	void run() const;

private:
	SocketHandle 	create_listen_socket() const;
	void 			bind_and_listen(const SocketHandle& server_socket) const;
	void 			serve_clients(const SocketHandle& server_socket, const ListenSocketStopSignals& stop_signals) const;
	
	static void log_client_connected(const ClientConnection& client_connection);
	static void run_chat_session(const ClientConnection& first, const ClientConnection& second);
	static void relay_messages(const ClientConnection& from, const ClientConnection& to);

	/** Must stay in sync with {@code MessengerClient::max_payload_bytes}. */
	static constexpr std::size_t MaxFramePayloadBytes = 1u << 20;
	static constexpr int Backlog = 5;
};
