#pragma once

#include "serverconfig.h"

#include <CLI/CLI.hpp>

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <string>


namespace will {


class ServerCliApp {
public:
    explicit ServerCliApp(const ServerConfig& defaults = ServerConfig{});

    void print_help(std::ostream& os) const;

    [[noreturn]] void exit_on_help(const CLI::CallForHelp& error) const;

    [[noreturn]] void exit_on_parse_error(const CLI::ParseError& error) const;

    ServerConfig parse(int argc, char* argv[]);

private:
    void apply_to(ServerConfig& config) const;

    CLI::App app_;

    int port_;
    int io_threads_;
    int listen_backlog_;
    std::size_t max_clients_;
    std::string db_path_;
    int otp_ttl_sec_;
    int otp_length_;
    int max_verify_attempts_;
    int otp_cooldown_sec_;
    int max_requests_per_ip_;
    int ip_rate_window_sec_;
    std::string otp_hash_salt_;
    std::optional<std::string> dev_fixed_otp_;
    bool log_otp_for_dev_;
    int auth_pending_timeout_sec_;
};


} // namespace will
