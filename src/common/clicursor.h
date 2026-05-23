#pragma once

#include <cstddef>
#include <exception>
#include <optional>
#include <string_view>


namespace will {


/** Cursor over argv with value parsing helpers. */
class CliCursor {
public:
    CliCursor(int argc, char* argv[]);

    void begin_options() noexcept;
    [[nodiscard]] bool has_option() const noexcept;
    void next_option() noexcept;

    [[nodiscard]] std::string_view current_option() const;

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


} // namespace will
