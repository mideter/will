#pragma once

#include <optional>
#include <string_view>


namespace will {
namespace cli {


/** Cursor over argv option flags with value parsing helpers. */
class OptionCursor {
public:
    OptionCursor(int argc, char* argv[]);

    bool has_option() const noexcept;
    OptionCursor operator++(int) noexcept;

    std::string_view current_option() const;

    std::string_view need_value(std::string_view flag);
    int require_int(std::string_view flag);
    std::size_t require_size(std::string_view flag);

    [[noreturn]] void fail_flag(std::string_view flag) const;

private:
    static std::optional<std::size_t> parse_size(std::string_view text);
    static std::optional<int> parse_int(std::string_view text);

    int argc_;
    char** argv_;
    int index_ = 1;
    bool current_option_has_value_ = false;
};


} // namespace cli
} // namespace will
