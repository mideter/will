#include "listensocketstopsignals.h"

#include <sys/socket.h>

#include <cerrno>
#include <system_error>


namespace will {


volatile sig_atomic_t ListenSocketStopSignals::shutting_down_ = 0;
volatile int ListenSocketStopSignals::listen_fd_ = -1;
volatile sig_atomic_t ListenSocketStopSignals::chat_peer_a_fd_ = -1;
volatile sig_atomic_t ListenSocketStopSignals::chat_peer_b_fd_ = -1;


void ListenSocketStopSignals::invoke_stop_signal() noexcept
{
	shutting_down_ = 1;
	const int listen_fd = listen_fd_;
	if (listen_fd >= 0)
		::shutdown(listen_fd, SHUT_RDWR);

	const int peer_a = chat_peer_a_fd_;
	const int peer_b = chat_peer_b_fd_;
	if (peer_a >= 0)
		::shutdown(peer_a, SHUT_RDWR);
	if (peer_b >= 0)
		::shutdown(peer_b, SHUT_RDWR);
}


extern "C" void listen_socket_stop_signal_trampoline(int) noexcept
{
	ListenSocketStopSignals::invoke_stop_signal();
}


ListenSocketStopSignals::ListenSocketStopSignals(int listen_fd)
{
	listen_fd_ = listen_fd;

	struct sigaction sa {};
	sa.sa_handler = listen_socket_stop_signal_trampoline;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGTERM, &sa, &old_term_) != 0) {
		listen_fd_ = -1;
		throw std::system_error(errno, std::generic_category(), "sigaction(SIGTERM) failed");
	}

	if (sigaction(SIGINT, &sa, &old_int_) != 0) {
		sigaction(SIGTERM, &old_term_, nullptr);
		listen_fd_ = -1;
		throw std::system_error(errno, std::generic_category(), "sigaction(SIGINT) failed");
	}
}


ListenSocketStopSignals::~ListenSocketStopSignals()
{
	chat_peer_a_fd_ = -1;
	chat_peer_b_fd_ = -1;
	sigaction(SIGTERM, &old_term_, nullptr);
	sigaction(SIGINT, &old_int_, nullptr);
	listen_fd_ = -1;
	shutting_down_ = 0;
}


bool ListenSocketStopSignals::shutdown_requested() const noexcept
{
	return shutting_down_ != 0;
}


void ListenSocketStopSignals::set_chat_peer_fds(int peer_a_fd, int peer_b_fd) noexcept
{
	chat_peer_a_fd_ = static_cast<sig_atomic_t>(peer_a_fd);
	chat_peer_b_fd_ = static_cast<sig_atomic_t>(peer_b_fd);
}


} // namespace will
