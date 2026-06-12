#pragma once

#include "support/phone_number.h"

#include <string_view>


namespace will::domain {


class SmsSender {
public:
    virtual ~SmsSender() = default;

    virtual void send_otp(const PhoneNumber& phone, std::string_view code) = 0;
};


} // namespace will::domain
