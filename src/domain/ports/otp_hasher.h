#pragma once

#include <string>
#include <string_view>


namespace will::domain {


class OtpHasher {
public:
    virtual ~OtpHasher() = default;

    virtual std::string hash(std::string_view code, std::string_view salt) const = 0;
};


} // namespace will::domain
