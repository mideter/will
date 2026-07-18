#pragma once

#include "serverconfig.h"

#include <CLI/CLI.hpp>

#include <cstddef>
#include <iosfwd>
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
    int heartbeat_interval_seconds_;
    int heartbeat_timeout_seconds_;
};


} // namespace will
