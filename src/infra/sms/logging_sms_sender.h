#pragma once

#include "ports/sms_sender.h"

#include <string_view>


namespace will {


/** Development SMS adapter; logs OTP codes to stderr only when enabled. */
class LoggingSmsSender final : public domain::SmsSender {
public:
    explicit LoggingSmsSender(bool log_otp_for_dev = false);

    void send_otp(const domain::PhoneNumber& phone, std::string_view code) override;

private:
    bool log_otp_for_dev_;
};


} // namespace will
