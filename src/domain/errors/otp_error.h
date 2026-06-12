#pragma once


namespace will::domain {


enum class OtpError {
    InvalidPhone,
    RateLimited,
    InvalidCode,
    Expired,
    Internal,
};


} // namespace will::domain
