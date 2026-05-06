#pragma once

#include <csignal>


namespace will {


class ListenSocketStopSignals {
public:
	explicit ListenSocketStopSignals(int listen_fd);
	~ListenSocketStopSignals();

	ListenSocketStopSignals(const ListenSocketStopSignals&) = delete;
	ListenSocketStopSignals& operator=(const ListenSocketStopSignals&) = delete;

	bool shutdown_requested() const noexcept;

	/** fds to shutdown(SIGTERM handler) while clients are connected; use -1 for unused slot */
	static void set_chat_peer_fds(int peer_a_fd, int peer_b_fd) noexcept;

private:
	struct sigaction old_term_ {};
	struct sigaction old_int_ {};
};


} // namespace will
