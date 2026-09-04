#pragma once

#include "entities/dead_vessel.h"
#include "entities/heaven.h"
#include "entities/vessel.h"
#include "ids/soul.h"
#include "ids/vessel.h"

#include <mutex>
#include <optional>
#include <unordered_map>


namespace will::domain {


/// Earth (Земля) — runtime registry of vessels. Looks to Heaven; does not know Eternity.
class Earth {
public:
    explicit Earth(Heaven& heaven);

    std::optional<Vessel> find_by_dead(const DeadVessel& dead) const;
    std::optional<id::Soul> soul_id_for_dead(const DeadVessel& dead) const;

    void insert(Vessel vessel);

private:
    Heaven& heaven_;
    mutable std::mutex mutex_;
    std::unordered_map<id::Vessel, Vessel> vessels_by_id_;
    std::unordered_map<DeadVessel, id::Vessel> id_by_dead_;
};


} // namespace will::domain
