#pragma once

#include "entities/auth_token.h"

#include <optional>
#include <string_view>


namespace will::domain {


/// Validates opaque device tokens presented by clients.
class DeviceToken {
public:
    static constexpr std::size_t MinLength = 32;
    static constexpr std::size_t MaxLength = 128;

    static std::optional<DeviceToken> parse(std::string_view input);
    static AuthToken generate();

    [[nodiscard]] const AuthToken& value() const noexcept { return token_; }
    [[nodiscard]] std::string_view text() const noexcept { return token_.value; }

private:
    explicit DeviceToken(AuthToken token);

    AuthToken token_;
};


} // namespace will::domain
