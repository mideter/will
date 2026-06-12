#pragma once

#include "entities/message.h"
#include "entities/otp_challenge.h"

#include <cstddef>
#include <optional>
#include <string_view>


namespace will::domain {


class OtpStore {
public:
    virtual ~OtpStore() = default;

    virtual void save_challenge(OtpChallenge challenge) = 0;
    virtual std::optional<OtpChallenge> find_challenge(std::string_view phone) = 0;
    virtual void increment_attempts(std::string_view phone) = 0;
    virtual void invalidate(std::string_view phone) = 0;
    virtual std::size_t count_challenges_by_peer_ip(std::string_view peer_ip, TimestampMs since_ms) = 0;
};


} // namespace will::domain
