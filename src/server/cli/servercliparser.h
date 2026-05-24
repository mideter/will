#pragma once

#include "clioption.h"
#include "serverclioption.h"
#include "serverconfig.h"


namespace will {


class ServerCliParser {
public:
    ServerCliParser(int argc, char* argv[]);

    const ServerConfig& server_config() const noexcept { return server_config_; }

private:
    void parse_command_line(int argc, char* argv[]);

    static void handle_help_option(int argc, CliCursor& cursor);
    static void print_usage();
    void apply_cli_option(const CliOptionMatch<ServerOption>& match);

    ServerConfig server_config_;
};


} // namespace will
