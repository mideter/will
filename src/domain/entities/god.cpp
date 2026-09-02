#include "god.h"


namespace will::domain {


God::God(const GodId id, DeviceToken device_token, GodName name)
    : id_(id)
    , device_token_(std::move(device_token))
    , name_(std::move(name))
{
}


} // namespace will::domain
