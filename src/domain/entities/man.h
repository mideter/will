#pragma once

#include "entities/soul.h"
#include "entities/vessel.h"


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel.
class Man {
public:
    /// Throws std::invalid_argument if vessel.soul_id() != soul.id().
    Man(Soul&& soul, Vessel&& vessel);

    const Soul& soul() const noexcept { return soul_; }
    const Vessel& vessel() const noexcept { return vessel_; }

    bool operator==(const Man&) const = default;

private:
    Soul soul_;
    Vessel vessel_;
};


} // namespace will::domain
