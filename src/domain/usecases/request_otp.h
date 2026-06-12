#pragma once

#include "errors/otp_error.h"
#include "entities/message.h"
#include "entities/otp_challenge.h"
#include "ports/otp_hasher.h"
#include "support/phone_number.h"
#include "ports/otp_store.h"
#include "ports/sms_sender.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>


namespace will::domain {


struct RequestOtpInput {
    std::string_view phone_raw;
    std::string_view peer_ip;
    TimestampMs now_ms = 0;
};


struct RequestOtpConfig {
    int otp_ttl_sec = 300;
    int otp_length = 6;
    int otp_cooldown_sec = 60;
    int max_requests_per_ip = 10;
    int ip_rate_window_sec = 3600;
    std::string hash_salt;
    std::optional<std::string> dev_fixed_otp;
};


struct RequestOtpSuccess {};


class RequestOtp {
public:
    RequestOtp(OtpStore& otp_store, SmsSender& sms_sender, OtpHasher& hasher, RequestOtpConfig config);

    std::variant<RequestOtpSuccess, OtpError> execute(const RequestOtpInput& input);

private:
    [[nodiscard]] std::variant<PhoneNumber, OtpError> parse_phone(std::string_view phone_raw) const;
    [[nodiscard]] std::optional<OtpError> check_rate_limits(const PhoneNumber& phone, std::string_view peer_ip,
                                                              TimestampMs now_ms) const;
    [[nodiscard]] OtpChallenge build_challenge(const PhoneNumber& phone, std::string_view peer_ip, TimestampMs now_ms,
                                               std::string code_hash) const;
    void persist_and_send(const PhoneNumber& phone, OtpChallenge challenge, std::string_view code);

    OtpStore& otp_store_;
    SmsSender& sms_sender_;
    OtpHasher& hasher_;
    RequestOtpConfig config_;
};


} // namespace will::domain
