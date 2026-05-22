#pragma once

#include <cstddef>
#include <cstdint>


namespace will {


/** Server configuration with enforced field invariants. */
class ServerConfig {
public:
    static constexpr int MinListenPort = 1;
    static constexpr int MaxListenPort = 65535;
    static constexpr std::uint16_t DefaultListenPort = 7770;
    static constexpr int DefaultIoThreads = 4;
    static constexpr int DefaultListenBacklog = 1024;
    static constexpr std::size_t DefaultMaxConnections = 4096;
    static constexpr std::size_t DefaultMaxOutboundQueueBytes = 1u << 20; // 1 MiB per session

    ServerConfig();

    [[nodiscard]] std::uint16_t listen_port() const noexcept { return listen_port_; }
    [[nodiscard]] int io_threads() const noexcept { return io_threads_; }
    [[nodiscard]] int listen_backlog() const noexcept { return listen_backlog_; }
    [[nodiscard]] std::size_t max_connections() const noexcept { return max_connections_; }
    [[nodiscard]] std::size_t max_outbound_queue_bytes() const noexcept
    {
        return max_outbound_queue_bytes_;
    }

    void set_listen_port(int port);
    void set_io_threads(int threads);
    void set_listen_backlog(int backlog);
    void set_max_connections(std::size_t max_connections);
    void set_max_outbound_queue_bytes(std::size_t max_bytes);

private:
    std::uint16_t listen_port_ = DefaultListenPort;
    int io_threads_ = DefaultIoThreads;
    int listen_backlog_ = DefaultListenBacklog;
    std::size_t max_connections_ = DefaultMaxConnections;
    std::size_t max_outbound_queue_bytes_ = DefaultMaxOutboundQueueBytes;
};


} // namespace will
