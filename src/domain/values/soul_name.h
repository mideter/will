#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>


namespace will::domain {


/** Random public soul label: Latin letters a-z and digits, fixed length. */
class SoulName {
public:
    static constexpr std::size_t Length = 8;

    static std::optional<SoulName> parse(std::string_view input);
    static SoulName generate();

    std::string_view text() const noexcept { return value_; }

    bool operator==(const SoulName&) const = default;

private:
    explicit SoulName(std::string value);

    std::string value_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::SoulName> {
    std::size_t operator()(const will::domain::SoulName& name) const noexcept
    {
        return std::hash<std::string_view>{}(name.text());
    }
};
