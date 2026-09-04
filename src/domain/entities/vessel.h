#pragma once

#include "entities/dead_vessel.h"
#include "ids/soul.h"
#include "ids/vessel.h"


namespace will::domain {


/// Vessel (Сосуд) — device through which a soul reaches the world.
class Vessel {
public:
    Vessel(id::Vessel id, DeadVessel dead, id::Soul soul_id);

    id::Vessel id() const noexcept { return id_; }
    const DeadVessel& dead() const noexcept { return dead_; }
    id::Soul soul_id() const noexcept { return soul_id_; }

    bool operator==(const Vessel&) const = default;

private:
    id::Vessel id_;
    DeadVessel dead_;
    id::Soul soul_id_;
};


} // namespace will::domain
