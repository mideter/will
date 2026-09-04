#include "vessel.h"

#include <utility>


namespace will::domain {


Vessel::Vessel(const id::Vessel id, DeadVessel dead, const id::Soul soul_id)
    : id_(id)
    , dead_(std::move(dead))
    , soul_id_(soul_id)
{}


} // namespace will::domain
