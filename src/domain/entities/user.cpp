#include "user.h"

#include <stdexcept>


namespace will::domain {


User::User(const UserId id, DeviceToken device_token, UserName name)
    : id_(id)
    , device_token_(std::move(device_token))
    , name_(std::move(name))
{
}


} // namespace will::domain
