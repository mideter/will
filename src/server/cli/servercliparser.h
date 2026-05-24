#pragma once

#include "clioption.h"
#include "serverconfig.h"


namespace will {


class CliCursor;


class ServerCliParser {
public:
    ServerCliParser(int argc, char* argv[]);

    const ServerConfig& server_config() const noexcept { return server_config_; }

private:
    void parse_command_line(int argc, char* argv[]);

    static void handle_help_option(int argc, CliCursor& cursor);
    static void print_usage();

    [[noreturn]] static void exit_with_help();

    ServerConfig server_config_;
};


} // namespace will
