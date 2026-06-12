#include "request_otp.h"

#include "support/phone_number.h"

#include <random>


namespace will::domain {


namespace {


std::string generate_otp_code(const int length, const std::optional<std::string>& dev_fixed_otp)
{
    if (dev_fixed_otp)
        return *dev_fixed_otp;

    thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> digit_dist(0, 9);

    std::string code;
    code.reserve(static_cast<std::size_t>(length));
    for (int i = 0; i < length; ++i)
        code.push_back(static_cast<char>('0' + digit_dist(rng)));
    return code;
}


bool is_within_cooldown(const OtpChallenge& challenge, const TimestampMs now_ms, const int otp_ttl_sec,
                        const int otp_cooldown_sec)
{
    const TimestampMs created_at_ms = challenge.expires_at_ms - static_cast<TimestampMs>(otp_ttl_sec) * 1000;
    return now_ms < created_at_ms + static_cast<TimestampMs>(otp_cooldown_sec) * 1000;
}


} // namespace


RequestOtp::RequestOtp(OtpStore& otp_store, SmsSender& sms_sender, OtpHasher& hasher, RequestOtpConfig config)
    : otp_store_(otp_store)
    , sms_sender_(sms_sender)
    , hasher_(hasher)
    , config_(std::move(config))
{}


std::variant<RequestOtpSuccess, OtpError> RequestOtp::execute(const RequestOtpInput& input)
{
    const auto phone_or_error = parse_phone(input.phone_raw);
    if (std::holds_alternative<OtpError>(phone_or_error))
        return std::get<OtpError>(phone_or_error);

    const PhoneNumber& phone = std::get<PhoneNumber>(phone_or_error);
    if (const std::optional<OtpError> rate_error = check_rate_limits(phone, input.peer_ip, input.now_ms))
        return *rate_error;

    const std::string code = generate_otp_code(config_.otp_length, config_.dev_fixed_otp);
    persist_and_send(phone, build_challenge(phone, input.peer_ip, input.now_ms, hasher_.hash(code, config_.hash_salt)),
                     code);

    return RequestOtpSuccess{};
}


std::variant<PhoneNumber, OtpError> RequestOtp::parse_phone(const std::string_view phone_raw) const
{
    if (config_.hash_salt.empty())
        return OtpError::Internal;

    const auto phone = PhoneNumber::parse(phone_raw);
    if (!phone)
        return OtpError::InvalidPhone;

    return *phone;
}


std::optional<OtpError> RequestOtp::check_rate_limits(const PhoneNumber& phone, const std::string_view peer_ip,
                                                      const TimestampMs now_ms) const
{
    if (const std::optional<OtpChallenge> existing = otp_store_.find_challenge(phone.e164())) {
        if (existing->expires_at_ms > now_ms
            && is_within_cooldown(*existing, now_ms, config_.otp_ttl_sec, config_.otp_cooldown_sec))
            return OtpError::RateLimited;
    }

    const TimestampMs since_ms = now_ms - static_cast<TimestampMs>(config_.ip_rate_window_sec) * 1000;
    if (otp_store_.count_challenges_by_peer_ip(peer_ip, since_ms) >=
        static_cast<std::size_t>(config_.max_requests_per_ip))
        return OtpError::RateLimited;

    return std::nullopt;
}


OtpChallenge RequestOtp::build_challenge(const PhoneNumber& phone, const std::string_view peer_ip,
                                         const TimestampMs now_ms, std::string code_hash) const
{
    OtpChallenge challenge;
    challenge.phone = phone.e164();
    challenge.code_hash = std::move(code_hash);
    challenge.expires_at_ms = now_ms + static_cast<TimestampMs>(config_.otp_ttl_sec) * 1000;
    challenge.attempts = 0;
    challenge.peer_ip = std::string(peer_ip);
    return challenge;
}


void RequestOtp::persist_and_send(const PhoneNumber& phone, OtpChallenge challenge, const std::string_view code)
{
    otp_store_.save_challenge(std::move(challenge));
    sms_sender_.send_otp(phone, code);
}


} // namespace will::domain
