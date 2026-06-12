#pragma once

#include "message.h"

#include <cstdint>
#include <string>


namespace will::domain {


struct OtpChallenge {
    std::string phone;
    std::string code_hash;
    TimestampMs expires_at_ms = 0;
    std::int32_t attempts = 0;
    std::string peer_ip;
};


} // namespace will::domain
