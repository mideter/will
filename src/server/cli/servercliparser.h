#pragma once

#include "clioption.h"
#include "serverclioption.h"
#include "serverclioptioncursor.h"
#include "serverconfig.h"


namespace will {
namespace cli {


class ServerParser {
public:
    ServerParser(int argc, char* argv[]);

    const ServerConfig& server_config() const noexcept;

private:
    void parse_command_line(int argc, char* argv[]);

    static void handle_help_option(int argc, ServerOptionCursor& cursor);
    static void print_usage();
    void apply_option(const OptionMatch<ServerOption>& match);

    ServerConfig server_config_;
};


} // namespace cli
} // namespace will
