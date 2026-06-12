#include "logging_sms_sender.h"

#include <cstdio>


namespace will {


LoggingSmsSender::LoggingSmsSender(const bool log_otp_for_dev)
    : log_otp_for_dev_(log_otp_for_dev)
{}


void LoggingSmsSender::send_otp(const domain::PhoneNumber& phone, const std::string_view code)
{
    if (!log_otp_for_dev_)
        return;

    std::fprintf(stderr, "[dev] OTP for %s: %.*s\n", phone.e164().c_str(),
                 static_cast<int>(code.size()), code.data());
}


} // namespace will
