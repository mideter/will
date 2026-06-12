#pragma once

#include "entities/auth_token.h"
#include "entities/message.h"
#include "entities/otp_challenge.h"
#include "entities/user_id.h"
#include "errors/otp_error.h"
#include "ports/auth_session_store.h"
#include "ports/otp_hasher.h"
#include "ports/otp_store.h"
#include "ports/user_repository.h"
#include "support/phone_number.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>


namespace will::domain {


struct VerifyOtpInput {
    PhoneNumber phone;
    std::string_view code;
    TimestampMs now_ms = 0;
};


struct VerifyOtpConfig {
    int max_verify_attempts = 5;
    std::string hash_salt;
};


struct VerifyOtpSuccess {
    AuthToken token;
    UserId user_id{};
};


class VerifyOtp {
public:
    VerifyOtp(UserRepository& users, OtpStore& otp_store, AuthSessionStore& sessions, OtpHasher& hasher,
              VerifyOtpConfig config);

    std::variant<VerifyOtpSuccess, OtpError> execute(const VerifyOtpInput& input);

private:
    [[nodiscard]] std::variant<OtpChallenge, OtpError> load_challenge(const PhoneNumber& phone,
                                                                        TimestampMs now_ms);
    [[nodiscard]] std::optional<OtpError> verify_code(const OtpChallenge& challenge, const PhoneNumber& phone,
                                                      std::string_view code);
    [[nodiscard]] VerifyOtpSuccess issue_session(const PhoneNumber& phone);

    UserRepository& users_;
    OtpStore& otp_store_;
    AuthSessionStore& sessions_;
    OtpHasher& hasher_;
    VerifyOtpConfig config_;
};


} // namespace will::domain
