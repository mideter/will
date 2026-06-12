#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
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

    static constexpr int DefaultOtpTtlSec = 300;
    static constexpr int DefaultOtpLength = 6;
    static constexpr int DefaultMaxVerifyAttempts = 5;
    static constexpr int DefaultOtpCooldownSec = 60;
    static constexpr int DefaultMaxRequestsPerIp = 10;
    static constexpr int DefaultIpRateWindowSec = 3600;
    static constexpr int DefaultAuthPendingTimeoutSec = 300;

    std::uint16_t listen_port = DefaultListenPort;
    int io_threads = DefaultIoThreads;
    int listen_backlog = DefaultListenBacklog;
    std::size_t max_connections = DefaultMaxConnections;
    std::string db_path = DefaultDbPath;

    int otp_ttl_sec = DefaultOtpTtlSec;
    int otp_length = 6;
    int max_verify_attempts = DefaultMaxVerifyAttempts;
    int otp_cooldown_sec = DefaultOtpCooldownSec;
    int max_requests_per_ip = DefaultMaxRequestsPerIp;
    int ip_rate_window_sec = DefaultIpRateWindowSec;
    std::string otp_hash_salt;
    std::optional<std::string> dev_fixed_otp;
    bool log_otp_for_dev = false;
    int auth_pending_timeout_sec = DefaultAuthPendingTimeoutSec;
};


} // namespace will
