#pragma once

#include <atomic>
#include <csignal>


namespace will {


/** `extern "C"` so `sigaction` gets a plain C handler; matches the `friend` declaration below. */
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

    static constexpr int MaxRegisteredChatPeerFds = 64;

    /**
     * Register a chat client socket fd for graceful shutdown(SIGINT/SIGTERM).
     * @return Slot index (&gt;= 0) on success, or -1 if the registry is full.
     */
    static int register_chat_peer_fd(int fd) noexcept;

    static void unregister_chat_peer_fd(int slot) noexcept;

private:
    /** POSIX handler is C function pointer; trampoline calls {@link invoke_stop_signal}. */
    friend void listen_socket_stop_signal_trampoline(int) noexcept;

    static void invoke_stop_signal() noexcept;

    static volatile sig_atomic_t shutting_down_;
    static volatile int listen_fd_;
    static std::atomic<int> chat_peer_fd_slots_[MaxRegisteredChatPeerFds];

    struct sigaction old_term_ {};
    struct sigaction old_int_ {};
};


} // namespace will
