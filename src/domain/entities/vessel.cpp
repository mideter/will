#include "vessel.h"

#include <utility>


namespace will::domain {


Vessel::Vessel(const id::Vessel id, DeviceToken token)
    : id_(id)
    , token_(std::move(token))
{}


} // namespace will::domain
