#include "vessel.h"


namespace will::domain {


Vessel::Vessel(const id::Vessel id, DeviceToken token, const id::God god_id)
    : id_(id)
    , token_(std::move(token))
    , god_id_(god_id)
{}


} // namespace will::domain
