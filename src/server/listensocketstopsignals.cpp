#include "listensocketstopsignals.h"

#include <sys/socket.h>

#include <cerrno>
#include <mutex>
#include <system_error>


namespace will {


volatile sig_atomic_t ListenSocketStopSignals::shutting_down_ = 0;
volatile int ListenSocketStopSignals::listen_fd_ = -1;
std::atomic<int> ListenSocketStopSignals::chat_peer_fd_slots_[ListenSocketStopSignals::max_registered_chat_peer_fds];


void ListenSocketStopSignals::invoke_stop_signal() noexcept
{
	shutting_down_ = 1;
	const int listen_fd = listen_fd_;
	if (listen_fd >= 0)
		::shutdown(listen_fd, SHUT_RDWR);

	for (int i = 0; i < max_registered_chat_peer_fds; ++i) {
		const int fd = chat_peer_fd_slots_[i].load();
		if (fd >= 0)
			::shutdown(fd, SHUT_RDWR);
	}
}


extern "C" void listen_socket_stop_signal_trampoline(int) noexcept
{
	ListenSocketStopSignals::invoke_stop_signal();
}


ListenSocketStopSignals::ListenSocketStopSignals(int listen_fd)
{
	static std::once_flag slots_initialized;
	std::call_once(slots_initialized, []() noexcept {
		for (int i = 0; i < max_registered_chat_peer_fds; ++i)
			chat_peer_fd_slots_[i].store(-1);
	});

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
	for (int i = 0; i < max_registered_chat_peer_fds; ++i)
		chat_peer_fd_slots_[i].store(-1);

	sigaction(SIGTERM, &old_term_, nullptr);
	sigaction(SIGINT, &old_int_, nullptr);
	listen_fd_ = -1;
	shutting_down_ = 0;
}


bool ListenSocketStopSignals::shutdown_requested() const noexcept
{
	return shutting_down_ != 0;
}


int ListenSocketStopSignals::register_chat_peer_fd(int fd) noexcept
{
	if (fd < 0)
		return -1;

	for (int i = 0; i < max_registered_chat_peer_fds; ++i) {
		int expected = -1;
		if (chat_peer_fd_slots_[i].compare_exchange_strong(expected, fd))
			return i;
	}

	return -1;
}


void ListenSocketStopSignals::unregister_chat_peer_fd(int slot) noexcept
{
	if (slot >= 0 && slot < max_registered_chat_peer_fds)
		chat_peer_fd_slots_[slot].store(-1);
}


} // namespace will
