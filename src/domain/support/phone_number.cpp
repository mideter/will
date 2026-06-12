#include "support/phone_number.h"

#include <algorithm>
#include <cctype>


namespace will::domain {


namespace {


bool is_ascii_digit(const char c) noexcept { return c >= '0' && c <= '9'; }


std::string strip_formatting(std::string_view input)
{
    std::string out;
    out.reserve(input.size());
    for (const char c : input) {
        if (c == ' ' || c == '-' || c == '(' || c == ')')
            continue;
        out.push_back(c);
    }
    return out;
}


std::optional<std::string> normalize_to_e164(std::string_view input)
{
    std::string normalized = strip_formatting(input);
    if (normalized.empty())
        return std::nullopt;

    if (normalized.rfind("00", 0) == 0)
        normalized.replace(0, 2, "+");

    if (normalized[0] != '+') {
        if (!std::all_of(normalized.begin(), normalized.end(), is_ascii_digit))
            return std::nullopt;
        normalized.insert(normalized.begin(), '+');
    }

    if (normalized.size() < 2u || normalized[0] != '+')
        return std::nullopt;

    const std::string_view digits = std::string_view{normalized}.substr(1);
    if (digits.size() < 8u || digits.size() > 15u)
        return std::nullopt;
    if (!is_ascii_digit(digits[0]) || digits[0] == '0')
        return std::nullopt;
    if (!std::all_of(digits.begin(), digits.end(), is_ascii_digit))
        return std::nullopt;

    return normalized;
}


} // namespace


PhoneNumber::PhoneNumber(std::string e164) : e164_(std::move(e164)) {}


std::optional<PhoneNumber> PhoneNumber::parse(const std::string_view input)
{
    const auto normalized = normalize_to_e164(input);
    if (!normalized)
        return std::nullopt;
    return PhoneNumber{*normalized};
}


} // namespace will::domain
