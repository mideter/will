#pragma once

#include <iosfwd>
#include <optional>
#include <span>
#include <string_view>
#include <vector>


namespace will {


class CliCursor;


enum class CliValueType {
    None,
    Int,
    Size,
};


class CliOption {
public:
    virtual ~CliOption() = default;

    [[nodiscard]] virtual bool matches(std::string_view text) const;
    [[nodiscard]] virtual std::string_view primary_flag() const;
    virtual void print_usage(std::ostream& os) const = 0;

    [[nodiscard]] CliValueType value_type() const noexcept { return value_type_; }

    [[nodiscard]] static std::span<const CliOption* const> all_options();

protected:
    explicit CliOption(std::string_view flag, CliValueType value_type);

    [[nodiscard]] std::string_view flag() const noexcept;

private:
    inline static std::vector<const CliOption*> registered_options_{};

    std::string_view flag_;
    CliValueType value_type_;
};


class CliOptionMatch {
public:
    CliOptionMatch(const CliOption* option, std::string_view token) noexcept
        : option_(option)
        , token_(token)
    {}

    explicit operator bool() const noexcept { return option_ != nullptr; }

    void read_value(CliCursor& cursor);

    [[nodiscard]] std::string_view token() const noexcept { return token_; }
    [[nodiscard]] std::string_view primary_flag() const { return option_->primary_flag(); }

    [[nodiscard]] std::optional<int> int_value() const noexcept { return int_value_; }
    [[nodiscard]] std::optional<std::size_t> size_value() const noexcept { return size_value_; }

private:
    const CliOption* option_;
    std::string_view token_;
    std::optional<int> int_value_;
    std::optional<std::size_t> size_value_;
};


} // namespace will
