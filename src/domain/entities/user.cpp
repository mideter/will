#include "user.h"

#include <stdexcept>


namespace will::domain {


User::User(const UserId id, DeviceToken device_token, UserName name)
    : id_(id)
    , device_token_(std::move(device_token))
    , name_(std::move(name))
{
    if (id_.value == 0)
        throw std::invalid_argument("User id must be non-zero");
}


} // namespace will::domain
