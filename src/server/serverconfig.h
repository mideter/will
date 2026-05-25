#pragma once

#include <cstddef>
#include <cstdint>


namespace will {


/** Server configuration (no invariants; validated by WillServer::validate_config). */
struct ServerConfig {
    static constexpr int MinListenPort = 1;
    static constexpr int MaxListenPort = 65535;
    static constexpr std::uint16_t DefaultListenPort = 7770;
    static constexpr int DefaultIoThreads = 4;
    static constexpr int DefaultListenBacklog = 1024;
    static constexpr std::size_t DefaultMaxConnections = 4096;
    static constexpr std::size_t DefaultMaxOutboundQueueBytes = 1u << 20; // 1 MiB per session

    std::uint16_t listen_port = DefaultListenPort;
    int io_threads = DefaultIoThreads;
    int listen_backlog = DefaultListenBacklog;
    std::size_t max_connections = DefaultMaxConnections;
    std::size_t max_outbound_queue_bytes = DefaultMaxOutboundQueueBytes;
};


} // namespace will
