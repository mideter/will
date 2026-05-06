#pragma once

#include <csignal>


namespace will {


extern "C" void listen_socket_stop_signal_trampoline(int) noexcept;


class ListenSocketStopSignals {
public:
	explicit ListenSocketStopSignals(int listen_fd);
	~ListenSocketStopSignals();

	ListenSocketStopSignals(const ListenSocketStopSignals&) = delete;
	ListenSocketStopSignals& operator=(const ListenSocketStopSignals&) = delete;
	ListenSocketStopSignals(ListenSocketStopSignals&&) = delete;
	ListenSocketStopSignals& operator=(ListenSocketStopSignals&&) = delete;

	bool shutdown_requested() const noexcept;

	/** fds to shutdown(SIGTERM handler) while clients are connected; use -1 for unused slot */
	static void set_chat_peer_fds(int peer_a_fd, int peer_b_fd) noexcept;

private:
	/** POSIX handler is C function pointer; trampoline calls {@link invoke_stop_signal}. */
	friend void listen_socket_stop_signal_trampoline(int) noexcept;

	static void invoke_stop_signal() noexcept;

	static volatile sig_atomic_t shutting_down_;
	static volatile int listen_fd_;
	static volatile sig_atomic_t chat_peer_a_fd_;
	static volatile sig_atomic_t chat_peer_b_fd_;

	struct sigaction old_term_ {};
	struct sigaction old_int_ {};
};


} // namespace will
