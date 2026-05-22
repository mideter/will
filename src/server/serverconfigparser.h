#pragma once

#include "serverconfig.h"
#include "serverconfigerror.h"

#include <cstddef>
#include <optional>
#include <string_view>


namespace will {


class ServerConfigParser {
public:
    ServerConfigParser(int argc, char* argv[]);

    [[nodiscard]] const ServerConfig& config() const noexcept { return config_; }

    static void print_usage();

private:
    void parse_command_line();

    [[nodiscard]] std::string_view need_value(const char* flag);

    static std::optional<std::size_t> parse_size(std::string_view text);
    static std::optional<int> parse_int(std::string_view text);

    [[noreturn]] void cli_fail(const char* message) const;
    [[noreturn]] void cli_fail_option(std::string_view option, const ServerConfigError& error) const;

    void apply_port();
    void apply_io_threads();
    void apply_listen_backlog();
    void apply_max_clients();
    void apply_max_outbound_queue_bytes();

    void apply_option(std::string_view option);

    ServerConfig config_;
    int argc_;
    char** argv_;
    int index_ = 0;
};


} // namespace will
