#pragma once

#include "ports/otp_hasher.h"


namespace will {


/** SHA-256(salt || code) hex digest for OTP challenge storage. */
class Sha256OtpHasher final : public domain::OtpHasher {
public:
    std::string hash(std::string_view code, std::string_view salt) const override;
};


} // namespace will
