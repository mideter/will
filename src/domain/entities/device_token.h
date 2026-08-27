#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>


namespace will::domain {


/// Validates opaque device tokens presented by clients.
class DeviceToken {
public:
    static constexpr std::size_t MinLength = 32;
    static constexpr std::size_t MaxLength = 128;

    static std::optional<DeviceToken> parse(std::string_view input);
    static DeviceToken generate();

    std::string_view text() const noexcept { return value_; }

    bool operator==(const DeviceToken&) const = default;

private:
    explicit DeviceToken(std::string value);

    std::string value_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::DeviceToken> {
    std::size_t operator()(const will::domain::DeviceToken& token) const noexcept
    {
        return std::hash<std::string_view>{}(token.text());
    }
};
