#pragma once

#include <cstddef>
#include <vector>

#include "clientaddress.h"
#include "clientconnection.h"


namespace will {


/** Will protocol over a transport: {@link TcpFrame} length-prefixed payloads. */
class Client {
public:
	explicit Client(ClientConnection connection, int chat_peer_signal_slot = -1);

	Client(const Client&) = delete;
	Client& operator=(const Client&) = delete;
	Client(Client&&) noexcept = default;
	Client& operator=(Client&&) noexcept = default;

	int socket_fd() const noexcept;
	/** Slot from {@link ListenSocketStopSignals::register_chat_peer_fd}, or -1 if none. */
	int chat_peer_signal_slot() const noexcept;
	const ClientAddress& address() const noexcept;
	void shutdown() const;

	/** @return {@code false} if peer closed cleanly before the first byte of the frame header */
	bool recv_frame(std::vector<char>& payload_out) const;

	void send_frame(const char* payload, std::size_t payload_len) const;
	void send_frame(const std::vector<char>& payload) const;

private:
	ClientConnection connection_;
	int chat_peer_signal_slot_;

	static bool recv_exact_relaxed_eof_before_first_byte(const ClientConnection& from,
														 char* data,
														 std::size_t len);
};


} // namespace will
