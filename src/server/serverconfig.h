#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>


namespace will {


class CliOptionMatch;


class ServerConfigError : public std::invalid_argument {
public:
    ServerConfigError(std::string_view field, std::string_view reason);
};


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

    [[nodiscard]] std::uint16_t listen_port() const noexcept;
    [[nodiscard]] int io_threads() const noexcept;
    [[nodiscard]] int listen_backlog() const noexcept;
    [[nodiscard]] std::size_t max_connections() const noexcept;
    [[nodiscard]] std::size_t max_outbound_queue_bytes() const noexcept;

    void set_listen_port(int port);
    void set_io_threads(int threads);
    void set_listen_backlog(int backlog);
    void set_max_connections(std::size_t max_connections);
    void set_max_outbound_queue_bytes(std::size_t max_bytes);

    void apply_cli_option(const CliOptionMatch& option);

private:
    void apply_cli_option(std::string_view flag, int value);
    void apply_cli_option(std::string_view flag, std::size_t value);

    std::uint16_t listen_port_ = DefaultListenPort;
    int io_threads_ = DefaultIoThreads;
    int listen_backlog_ = DefaultListenBacklog;
    std::size_t max_connections_ = DefaultMaxConnections;
    std::size_t max_outbound_queue_bytes_ = DefaultMaxOutboundQueueBytes;
};


} // namespace will
