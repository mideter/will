#pragma once

#include <iosfwd>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>


namespace will {


class CliCursor;


enum class CliValueType {
    None,
    Int,
    Size,
};


class CliError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


class CliUnknownOptionError : public CliError {
public:
    explicit CliUnknownOptionError(std::string_view token);
};


class CliHelpNotAloneError : public CliError {
public:
    CliHelpNotAloneError();
};


class CliOption {
public:
    using UsagePrinter = void (*)(std::ostream&);

    CliOption(std::string_view flag, CliValueType value_type, UsagePrinter print_usage,
              std::span<const std::string_view> aliases = {});

    bool matches(std::string_view text) const;
    std::string_view primary_flag() const noexcept;
    void print_usage(std::ostream& os) const;
    CliValueType value_type() const noexcept;

private:
    std::string_view flag_;
    CliValueType value_type_;
    UsagePrinter print_usage_;
    std::span<const std::string_view> aliases_;
};


class CliOptionMatch {
public:
    static CliOptionMatch parse(CliCursor& cursor, std::span<const CliOption> options);

    std::string_view token() const noexcept { return token_; }
    std::string_view primary_flag() const { return option_->primary_flag(); }

    std::optional<int> int_value() const noexcept { return int_value_; }
    std::optional<std::size_t> size_value() const noexcept { return size_value_; }

private:
    CliOptionMatch(const CliOption& option, std::string_view token);

    void read_value(CliCursor& cursor);

    const CliOption* option_;
    std::string_view token_;
    std::optional<int> int_value_;
    std::optional<std::size_t> size_value_;
};


} // namespace will
