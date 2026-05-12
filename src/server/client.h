#pragma once

#include <cstddef>
#include <vector>

#include "clientaddress.h"
#include "clientconnection.h"
#include "connectionacceptor.h"


namespace will {


/** Will protocol over a transport: {@link TcpFrame} length-prefixed payloads. */
class Client {
public:
	explicit Client(AcceptedConnection accepted);

	Client(const Client&) = delete;
	Client& operator=(const Client&) = delete;
	Client(Client&&) noexcept = default;
	Client& operator=(Client&&) noexcept = default;

	const ClientAddress& address() const noexcept;
	void shutdown();

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
