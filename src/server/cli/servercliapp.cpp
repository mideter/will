#include "servercliapp.h"

#include "serverconfig.h"

#include <cstdlib>
#include <format>
#include <iostream>


namespace will {


ServerCliApp::ServerCliApp(const ServerConfig& defaults)
    : app_{"will-server"}
    , port_(static_cast<int>(defaults.listen_port))
    , io_threads_(defaults.io_threads)
    , listen_backlog_(defaults.listen_backlog)
    , max_clients_(defaults.max_connections)
    , db_path_(defaults.db_path)
    , otp_ttl_sec_(defaults.otp_ttl_sec)
    , otp_length_(defaults.otp_length)
    , max_verify_attempts_(defaults.max_verify_attempts)
    , otp_cooldown_sec_(defaults.otp_cooldown_sec)
    , max_requests_per_ip_(defaults.max_requests_per_ip)
    , ip_rate_window_sec_(defaults.ip_rate_window_sec)
    , otp_hash_salt_(defaults.otp_hash_salt)
    , log_otp_for_dev_(defaults.log_otp_for_dev)
    , auth_pending_timeout_sec_(defaults.auth_pending_timeout_sec)
{
    app_.allow_extras(false);

    app_.add_option("--port", port_)->description("Listen port");
    app_.add_option("--io-threads", io_threads_)->description("io_context worker threads");
    app_.add_option("--listen-backlog", listen_backlog_)->description("listen() backlog");
    app_.add_option("--max-clients", max_clients_)->description("Max concurrent connections");
    app_.add_option("--db-path", db_path_)->description("SQLite database path");
    app_.add_option("--otp-ttl-sec", otp_ttl_sec_)->description("OTP challenge TTL in seconds");
    app_.add_option("--otp-length", otp_length_)->description("OTP code length");
    app_.add_option("--max-verify-attempts", max_verify_attempts_)->description("Max OTP verify attempts");
    app_.add_option("--otp-cooldown-sec", otp_cooldown_sec_)->description("Min seconds between OTP requests");
    app_.add_option("--max-requests-per-ip", max_requests_per_ip_)->description("Max OTP requests per IP");
    app_.add_option("--ip-rate-window-sec", ip_rate_window_sec_)->description("OTP IP rate-limit window");
    app_.add_option("--otp-hash-salt", otp_hash_salt_)->description("Salt for OTP code hashing");
    app_.add_option("--dev-fixed-otp", dev_fixed_otp_)->description("Fixed OTP code for development");
    app_.add_flag("--log-otp-for-dev", log_otp_for_dev_)->description("Log OTP codes to stderr");
    app_.add_option("--auth-pending-timeout-sec", auth_pending_timeout_sec_)
        ->description("Seconds to wait for OtpCodeSubmit after OtpSent");
}


void ServerCliApp::print_help(std::ostream& os) const
{
    os << std::format(
        "Usage: will-server [options]\n"
        "\n"
        "Options:\n"
        "  -h, --help                      Print usage and exit\n"
        "  --port PORT                     Listen port (default {})\n"
        "  --io-threads N                  io_context worker threads (default {})\n"
        "  --listen-backlog N              listen() backlog (default {})\n"
        "  --max-clients N                 Max concurrent connections (default {})\n"
        "  --db-path PATH                  SQLite database path (default {})\n"
        "  --otp-ttl-sec N                 OTP challenge TTL (default {})\n"
        "  --otp-length N                  OTP code length (default {})\n"
        "  --max-verify-attempts N         Max OTP verify attempts (default {})\n"
        "  --otp-cooldown-sec N            Min seconds between OTP requests (default {})\n"
        "  --max-requests-per-ip N         Max OTP requests per IP (default {})\n"
        "  --ip-rate-window-sec N          OTP IP rate-limit window (default {})\n"
        "  --otp-hash-salt SALT            Salt for OTP code hashing\n"
        "  --dev-fixed-otp CODE            Fixed OTP code for development\n"
        "  --log-otp-for-dev               Log OTP codes to stderr\n"
        "  --auth-pending-timeout-sec N    OTP code submit timeout (default {})\n",
        ServerConfig::DefaultListenPort,
        ServerConfig::DefaultIoThreads,
        ServerConfig::DefaultListenBacklog,
        ServerConfig::DefaultMaxConnections,
        ServerConfig::DefaultDbPath,
        ServerConfig::DefaultOtpTtlSec,
        ServerConfig::DefaultOtpLength,
        ServerConfig::DefaultMaxVerifyAttempts,
        ServerConfig::DefaultOtpCooldownSec,
        ServerConfig::DefaultMaxRequestsPerIp,
        ServerConfig::DefaultIpRateWindowSec,
        ServerConfig::DefaultAuthPendingTimeoutSec);
}


void ServerCliApp::exit_on_help(const CLI::CallForHelp& error) const
{
    print_help(std::cerr);
    std::exit(error.get_exit_code());
}


void ServerCliApp::exit_on_parse_error(const CLI::ParseError& error) const
{
    std::exit(app_.exit(error, std::cerr));
}


void ServerCliApp::apply_to(ServerConfig& config) const
{
    config.listen_port = static_cast<std::uint16_t>(port_);
    config.io_threads = io_threads_;
    config.listen_backlog = listen_backlog_;
    config.max_connections = max_clients_;
    config.db_path = db_path_;
    config.otp_ttl_sec = otp_ttl_sec_;
    config.otp_length = otp_length_;
    config.max_verify_attempts = max_verify_attempts_;
    config.otp_cooldown_sec = otp_cooldown_sec_;
    config.max_requests_per_ip = max_requests_per_ip_;
    config.ip_rate_window_sec = ip_rate_window_sec_;
    config.otp_hash_salt = otp_hash_salt_;
    config.dev_fixed_otp = dev_fixed_otp_;
    config.log_otp_for_dev = log_otp_for_dev_;
    config.auth_pending_timeout_sec = auth_pending_timeout_sec_;
}


ServerConfig ServerCliApp::parse(int argc, char* argv[])
{
    ServerConfig config;
    app_.parse(argc, argv);
    apply_to(config);
    return config;
}


} // namespace will
