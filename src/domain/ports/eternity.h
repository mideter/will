#pragma once

#include "entities/god.h"
#include "entities/vessel.h"
#include "values/god_name.h"

#include <string_view>
#include <utility>
#include <vector>


namespace will::domain {


/// Eternal memory — durable storage. Runtime lookups use Heaven in memory.
class Eternity {
public:
    virtual ~Eternity() = default;

    virtual std::vector<God> load_gods() = 0;
    virtual std::vector<Vessel> load_vessels() = 0;
    virtual std::pair<God, Vessel> insert_god_with_vessel(std::string_view device_token, GodName name) = 0;
};


} // namespace will::domain
