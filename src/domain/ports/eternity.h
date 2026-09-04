#pragma once

#include "entities/dead_vessel.h"
#include "entities/soul.h"
#include "entities/vessel.h"
#include "values/soul_name.h"

#include <utility>
#include <vector>


namespace will::domain {


/// Eternal memory — durable storage. Runtime lookups use Heaven and Earth in memory.
class Eternity {
public:
    virtual ~Eternity() = default;

    virtual std::vector<Soul> load_souls() = 0;
    virtual std::vector<Vessel> load_vessels() = 0;
    virtual std::pair<Soul, Vessel> insert_soul_with_vessel(const DeadVessel& dead, SoulName name) = 0;
};


} // namespace will::domain
