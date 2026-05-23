#pragma once

#include <cstddef>
#include <exception>
#include <optional>
#include <string_view>


namespace will {


/** argv cursor and value parsing (no configuration target). */
class CliParserContextBase {
public:
    CliParserContextBase(int argc, char* argv[]);

    [[nodiscard]] int argc() const noexcept { return argc_; }
    [[nodiscard]] int index() const noexcept { return index_; }
    void set_index(int index) noexcept { index_ = index; }

    [[nodiscard]] std::string_view current() const;

    [[nodiscard]] std::string_view need_value(std::string_view flag);
    [[nodiscard]] int require_int(std::string_view flag);
    [[nodiscard]] std::size_t require_size(std::string_view flag);

    [[noreturn]] void cli_fail_flag(std::string_view flag) const;
    [[noreturn]] void cli_fail_option(std::string_view flag, const std::exception& error) const;

private:
    static std::optional<std::size_t> parse_size(std::string_view text);
    static std::optional<int> parse_int(std::string_view text);

    int argc_;
    char** argv_;
    int index_ = 0;
};


/** argv cursor bound to a configuration object of type Config. */
template<typename Config>
class CliParserContext : public CliParserContextBase {
public:
    CliParserContext(int argc, char* argv[], Config& config);

    [[nodiscard]] Config& config() noexcept { return config_; }
    [[nodiscard]] const Config& config() const noexcept { return config_; }

private:
    Config& config_;
};


template<typename Config>
CliParserContext<Config>::CliParserContext(int argc, char* argv[], Config& config)
    : CliParserContextBase(argc, argv)
    , config_(config)
{}


} // namespace will
