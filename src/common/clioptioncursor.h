#pragma once

#include <string_view>


namespace will {
namespace cli {


/** Cursor over argv option flags. */
class OptionCursor {
public:
    OptionCursor(int argc, char* argv[]);

    bool has_option() const noexcept;
    OptionCursor operator++(int) noexcept;

    std::string_view current_option() const;

private:
    friend class IntValue;
    friend class SizeValue;

    std::string_view need_value(std::string_view flag);

    int argc_;
    char** argv_;
    int index_ = 1;
    bool current_option_has_value_ = false;
};


} // namespace cli
} // namespace will
