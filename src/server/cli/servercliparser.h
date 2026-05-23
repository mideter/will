#pragma once

#include "clioption.h"
#include "serverconfig.h"

#include <exception>
#include <iosfwd>
#include <string_view>


namespace will {


class ServerCliParser {
public:
    ServerCliParser(int argc, char* argv[]);

    [[nodiscard]] const ServerConfig& server_config() const noexcept { return server_config_; }

    static void print_usage();

private:
    void parse_command_line(int argc, char* argv[]);

    static void print_option_usage(std::ostream& os);

    [[noreturn]] static void cli_fail_option(std::string_view flag, const std::exception& error);

    ServerConfig server_config_;
};


} // namespace will
