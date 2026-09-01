#include "account.h"

#include <stdexcept>


namespace will::domain {


Account::Account(const UserId user_id, DeviceToken device_token, const Timestamp authenticated_at)
    : user_id_(user_id)
    , device_token_(std::move(device_token))
    , authenticated_at_(authenticated_at)
{
    if (user_id_.value == 0)
        throw std::invalid_argument("Account user_id must be non-zero");
}


} // namespace will::domain
