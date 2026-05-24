#pragma once

#include "clioption.h"
#include "serverconfig.h"

#include <exception>
#include <string_view>


namespace will {


class CliCursor;


class ServerCliParser {
public:
    ServerCliParser(int argc, char* argv[]);

    const ServerConfig& server_config() const noexcept { return server_config_; }

private:
    void parse_command_line(int argc, char* argv[]);

    void apply_matched_option(const CliOptionMatch& option);

    static void handle_help_option(int argc, CliCursor& cursor);
    static void print_usage();

    [[noreturn]] static void fail_help_not_alone();
    [[noreturn]] static void exit_with_help();
    [[noreturn]] static void cli_fail_option(std::string_view flag, const std::exception& error);

    ServerConfig server_config_;
};


} // namespace will
