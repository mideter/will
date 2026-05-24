#pragma once

#include <array>
#include <cstddef>
#include <string_view>


namespace will {
namespace cli {


template<typename OptionVariant>
class OptionMatch;


/** Cursor over argv option flags. */
class OptionCursorCore {
public:
    OptionCursorCore(int argc, char* argv[]);

    bool has_option() const noexcept;
    OptionCursorCore operator++(int) noexcept;

    std::string_view current_option() const;

protected:
    friend class IntValue;
    friend class SizeValue;

    std::string_view need_value(std::string_view flag);

    int argc_;
    char** argv_;
    int index_ = 1;
    bool current_option_has_value_ = false;
};


template<typename OptionVariant, std::size_t N>
class OptionCursor : public OptionCursorCore {
public:
    OptionCursor(int argc, char* argv[], const std::array<OptionVariant, N>& options)
        : OptionCursorCore(argc, argv)
        , options_(options)
    {}

    OptionMatch<OptionVariant> match();
    void advance() { (*this)++; }

private:
    const std::array<OptionVariant, N>& options_;
};


template<typename OptionVariant, std::size_t N>
OptionCursor(int argc, char* argv[], const std::array<OptionVariant, N>& options)
    -> OptionCursor<OptionVariant, N>;


} // namespace cli
} // namespace will
