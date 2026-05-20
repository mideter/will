#pragma once

#include "serverconfig.h"

#include <optional>
#include <string_view>


namespace will {


class ServerConfigParser {
public:
    [[nodiscard]] static ServerConfig parse(int argc, char* argv[]);

private:
    static std::optional<std::size_t> parse_size(std::string_view text);
    static std::optional<int> parse_int(std::string_view text);
    static void print_usage();
};


} // namespace will
