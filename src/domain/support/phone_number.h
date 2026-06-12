#pragma once

#include <optional>
#include <string>
#include <string_view>


namespace will::domain {


/// Normalized E.164 phone number (e.g. {@code +15551234567}).
class PhoneNumber {
public:
    static std::optional<PhoneNumber> parse(std::string_view input);

    const std::string& e164() const noexcept { return e164_; }

    bool operator==(const PhoneNumber& other) const noexcept { return e164_ == other.e164_; }

private:
    explicit PhoneNumber(std::string e164);

    std::string e164_;
};


} // namespace will::domain
