#pragma once

#include "ids/man.h"
#include "ids/soul.h"
#include "ids/vessel.h"


namespace will::domain {


/// Man (Человек) — soul dwelling in a vessel. Identity and links are fixed after construction.
class Man {
public:
    Man(id::Man id, id::Soul soul_id, id::Vessel vessel_id);

    id::Man id() const noexcept { return id_; }
    id::Soul soul_id() const noexcept { return soul_id_; }
    id::Vessel vessel_id() const noexcept { return vessel_id_; }

    bool operator==(const Man&) const = default;

private:
    id::Man id_;
    id::Soul soul_id_;
    id::Vessel vessel_id_;
};


} // namespace will::domain
