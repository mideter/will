#include "verify_otp.h"

#include "entities/user.h"


namespace will::domain {


VerifyOtp::VerifyOtp(UserRepository& users, OtpStore& otp_store, AuthSessionStore& sessions, OtpHasher& hasher,
                     VerifyOtpConfig config)
    : users_(users)
    , otp_store_(otp_store)
    , sessions_(sessions)
    , hasher_(hasher)
    , config_(std::move(config))
{}


std::variant<VerifyOtpSuccess, OtpError> VerifyOtp::execute(const VerifyOtpInput& input)
{
    if (config_.hash_salt.empty())
        return OtpError::Internal;

    const auto challenge_or_error = load_challenge(input.phone, input.now_ms);
    if (std::holds_alternative<OtpError>(challenge_or_error))
        return std::get<OtpError>(challenge_or_error);

    const OtpChallenge& challenge = std::get<OtpChallenge>(challenge_or_error);
    if (const std::optional<OtpError> code_error = verify_code(challenge, input.phone, input.code))
        return *code_error;

    return issue_session(input.phone);
}


std::variant<OtpChallenge, OtpError> VerifyOtp::load_challenge(const PhoneNumber& phone, const TimestampMs now_ms)
{
    const std::optional<OtpChallenge> challenge = otp_store_.find_challenge(phone.e164());
    if (!challenge)
        return OtpError::InvalidCode;

    if (challenge->expires_at_ms <= now_ms) {
        otp_store_.invalidate(phone.e164());
        return OtpError::Expired;
    }

    if (challenge->attempts >= config_.max_verify_attempts) {
        otp_store_.invalidate(phone.e164());
        return OtpError::InvalidCode;
    }

    return *challenge;
}


std::optional<OtpError> VerifyOtp::verify_code(const OtpChallenge& challenge, const PhoneNumber& phone,
                                               const std::string_view code)
{
    if (hasher_.hash(code, config_.hash_salt) == challenge.code_hash)
        return std::nullopt;

    otp_store_.increment_attempts(phone.e164());
    return OtpError::InvalidCode;
}


VerifyOtpSuccess VerifyOtp::issue_session(const PhoneNumber& phone)
{
    std::optional<User> user = users_.find_by_phone(phone.e164());
    if (!user)
        user = users_.create_user(phone.e164());

    const AuthToken token = sessions_.issue_session(user->id);
    otp_store_.invalidate(phone.e164());

    return VerifyOtpSuccess{token, user->id};
}


} // namespace will::domain
