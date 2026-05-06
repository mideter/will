#include "listensocketstopsignals.h"

#include <sys/socket.h>

#include <cerrno>
#include <system_error>


namespace {


volatile sig_atomic_t g_shutting_down = 0;
volatile int g_listen_fd = -1;
volatile sig_atomic_t g_chat_peer_a_fd = -1;
volatile sig_atomic_t g_chat_peer_b_fd = -1;


void on_stop_signal(int)
{
	g_shutting_down = 1;
	const int listen_fd = g_listen_fd;
	if (listen_fd >= 0)
		::shutdown(listen_fd, SHUT_RDWR);

	const int peer_a = g_chat_peer_a_fd;
	const int peer_b = g_chat_peer_b_fd;
	if (peer_a >= 0)
		::shutdown(peer_a, SHUT_RDWR);
	if (peer_b >= 0)
		::shutdown(peer_b, SHUT_RDWR);
}


} // namespace


namespace will {


ListenSocketStopSignals::ListenSocketStopSignals(int listen_fd)
{
	g_listen_fd = listen_fd;

	struct sigaction sa {};
	sa.sa_handler = on_stop_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGTERM, &sa, &old_term_) != 0) {
		g_listen_fd = -1;
		throw std::system_error(errno, std::generic_category(), "sigaction(SIGTERM) failed");
	}

	if (sigaction(SIGINT, &sa, &old_int_) != 0) {
		sigaction(SIGTERM, &old_term_, nullptr);
		g_listen_fd = -1;
		throw std::system_error(errno, std::generic_category(), "sigaction(SIGINT) failed");
	}
}


ListenSocketStopSignals::~ListenSocketStopSignals()
{
	g_chat_peer_a_fd = -1;
	g_chat_peer_b_fd = -1;
	sigaction(SIGTERM, &old_term_, nullptr);
	sigaction(SIGINT, &old_int_, nullptr);
	g_listen_fd = -1;
	g_shutting_down = 0;
}


bool ListenSocketStopSignals::shutdown_requested() const noexcept
{
	return g_shutting_down != 0;
}


void ListenSocketStopSignals::set_chat_peer_fds(int peer_a_fd, int peer_b_fd) noexcept
{
	g_chat_peer_a_fd = static_cast<sig_atomic_t>(peer_a_fd);
	g_chat_peer_b_fd = static_cast<sig_atomic_t>(peer_b_fd);
}


} // namespace will
