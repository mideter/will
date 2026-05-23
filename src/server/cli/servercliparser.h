#pragma once

#include "clioption.h"
#include "serverconfig.h"

#include <iosfwd>
#include <string_view>


namespace will {


using ServerCliOption = CliOptionBase<ServerConfig>;


class ServerCliParser {
public:
    ServerCliParser(int argc, char* argv[]);

    [[nodiscard]] const ServerConfig& server_config() const noexcept { return server_config_; }

    static void print_usage();

private:
    void parse_command_line(int argc, char* argv[]);

    static void print_option_usage(std::ostream& os);
    [[nodiscard]] static const ServerCliOption* find_option(std::string_view text);

    ServerConfig server_config_;
};


} // namespace will
