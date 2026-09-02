#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>


namespace will::domain {


/** Random public god label: Latin letters a-z and digits, fixed length. */
class GodName {
public:
    static constexpr std::size_t Length = 8;

    static std::optional<GodName> parse(std::string_view input);
    static GodName generate();

    std::string_view text() const noexcept { return value_; }

    bool operator==(const GodName&) const = default;

private:
    explicit GodName(std::string value);

    std::string value_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::GodName> {
    std::size_t operator()(const will::domain::GodName& name) const noexcept
    {
        return std::hash<std::string_view>{}(name.text());
    }
};
