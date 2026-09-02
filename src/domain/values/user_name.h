#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>


namespace will::domain {


/** Random public user label: Latin letters a-z and digits, fixed length. */
class UserName {
public:
    static constexpr std::size_t Length = 8;

    static std::optional<UserName> parse(std::string_view input);
    static UserName generate();

    std::string_view text() const noexcept { return value_; }

    bool operator==(const UserName&) const = default;

private:
    explicit UserName(std::string value);

    std::string value_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::UserName> {
    std::size_t operator()(const will::domain::UserName& name) const noexcept
    {
        return std::hash<std::string_view>{}(name.text());
    }
};
