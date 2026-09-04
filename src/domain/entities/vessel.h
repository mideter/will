#pragma once

#include "entities/dead_vessel.h"
#include "ids/god.h"
#include "ids/vessel.h"


namespace will::domain {


/// Vessel (Сосуд) — device through which a god reaches the world.
class Vessel {
public:
    Vessel(id::Vessel id, DeadVessel dead, id::God god_id);

    id::Vessel id() const noexcept { return id_; }
    const DeadVessel& dead() const noexcept { return dead_; }
    id::God god_id() const noexcept { return god_id_; }

    bool operator==(const Vessel&) const = default;

private:
    id::Vessel id_;
    DeadVessel dead_;
    id::God god_id_;
};


} // namespace will::domain
