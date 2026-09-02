#include "vessel.h"


namespace will::domain {


Vessel::Vessel(const VesselId id, DeviceToken token, const GodId god_id)
    : id_(id)
    , token_(std::move(token))
    , god_id_(god_id)
{}


} // namespace will::domain
