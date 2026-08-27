#include "device_token.h"

#include <cctype>
#include <format>
#include <random>


namespace will::domain {


namespace {


bool is_valid_token_char(const char c)
{
    return std::isxdigit(static_cast<unsigned char>(c)) != 0;
}


} // namespace


std::optional<DeviceToken> DeviceToken::parse(const std::string_view input)
{
    if (input.size() < MinLength || input.size() > MaxLength)
        return std::nullopt;

    for (const char c : input) {
        if (!is_valid_token_char(c))
            return std::nullopt;
    }

    return DeviceToken{std::string(input)};
}


DeviceToken DeviceToken::generate()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<std::uint64_t> dist;
    return DeviceToken{std::format("{:016x}{:016x}", dist(gen), dist(gen))};
}


DeviceToken::DeviceToken(std::string value) : value_(std::move(value)) {}


} // namespace will::domain
