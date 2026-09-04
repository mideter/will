#include "vessel.h"

#include <utility>


namespace will::domain {


Vessel::Vessel(const id::Vessel id, DeadVessel dead, const id::God god_id)
    : id_(id)
    , dead_(std::move(dead))
    , god_id_(god_id)
{}


} // namespace will::domain
