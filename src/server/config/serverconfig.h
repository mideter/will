#pragma once

#include <cstddef>
#include <cstdint>
#include <string>


namespace will {


/** Server configuration (no invariants; validated by ServerConfigValidator). */
struct ServerConfig {
    static constexpr int MinListenPort = 1;
    static constexpr int MaxListenPort = 65535;
    static constexpr std::uint16_t DefaultListenPort = 7770;
    static constexpr int DefaultIoThreads = 4;
    static constexpr int DefaultListenBacklog = 1024;
    static constexpr std::size_t DefaultMaxConnections = 4096;
    static constexpr const char* DefaultDbPath = "will.db";
    static constexpr int DefaultHeartbeatIntervalSeconds = 30;
    static constexpr int DefaultHeartbeatTimeoutSeconds = 10;

    std::uint16_t listen_port = DefaultListenPort;
    int io_threads = DefaultIoThreads;
    int listen_backlog = DefaultListenBacklog;
    std::size_t max_connections = DefaultMaxConnections;
    std::string db_path = DefaultDbPath;
    int heartbeat_interval_seconds = DefaultHeartbeatIntervalSeconds;
    int heartbeat_timeout_seconds = DefaultHeartbeatTimeoutSeconds;
};


} // namespace will
