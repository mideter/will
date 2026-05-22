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
    enum class Option {
        Port,
        IoThreads,
        ListenBacklog,
        MaxClients,
        MaxOutboundQueueBytes,
        Help,
        Unknown
    };

    static Option classify_option(std::string_view option);
    static const char* option_flag(Option option);

    void parse_command_line();

    [[nodiscard]] std::string_view need_value(Option option);
    [[nodiscard]] int require_int(Option option);
    [[nodiscard]] std::size_t require_size(Option option);

    static std::optional<std::size_t> parse_size(std::string_view text);
    static std::optional<int> parse_int(std::string_view text);

    [[noreturn]] void cli_fail_flag(Option option) const;
    [[noreturn]] void cli_fail_option(Option option, const ServerConfigError& error) const;

    void apply_option(Option option);

    ServerConfig config_;
    int argc_;
    char** argv_;
    int index_ = 0;
};


} // namespace will
