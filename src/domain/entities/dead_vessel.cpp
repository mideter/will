#include "dead_vessel.h"

#include <optional>
#include <stdexcept>
#include <utility>


namespace will::domain {


namespace {


DeviceToken require_token(const std::string_view device_token_raw)
{
    if (const std::optional<DeviceToken> token = DeviceToken::parse(device_token_raw))
        return *token;

    throw std::invalid_argument("DeadVessel: invalid device token");
}


} // namespace


DeadVessel::DeadVessel(const std::string_view device_token_raw)
    : token_(require_token(device_token_raw))
{}


DeadVessel::DeadVessel(DeviceToken token)
    : token_(std::move(token))
{}


} // namespace will::domain
