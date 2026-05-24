#pragma once

#include <optional>
#include <string_view>


namespace will {


/** Cursor over argv with value parsing helpers. */
class CliCursor {
public:
    CliCursor(int argc, char* argv[]);

    void begin_options() noexcept;
    bool has_option() const noexcept;
    void next_option() noexcept;

    std::string_view current_option() const;

    std::string_view need_value(std::string_view flag);
    int require_int(std::string_view flag);
    std::size_t require_size(std::string_view flag);

    [[noreturn]] void cli_fail_flag(std::string_view flag) const;

private:
    static std::optional<std::size_t> parse_size(std::string_view text);
    static std::optional<int> parse_int(std::string_view text);

    int argc_;
    char** argv_;
    int index_ = 0;
};


} // namespace will
