#pragma once

#include "entities/man.h"
#include "entities/soul.h"
#include "entities/vessel.h"
#include "values/device_token.h"
#include "values/soul_name.h"

#include <vector>


namespace will::domain {


/// Result of creating a man in eternal memory.
struct ManBirth {
    Man man;
    Soul soul;
    Vessel vessel;
};


/// Eternal memory — durable storage. Runtime lookups use World (Heaven, Earth, men) in memory.
class Eternity {
public:
    virtual ~Eternity() = default;

    virtual std::vector<Soul> load_souls() = 0;
    virtual std::vector<Vessel> load_vessels() = 0;
    virtual std::vector<Man> load_men() = 0;
    virtual ManBirth insert_man(const DeviceToken& token, SoulName name) = 0;
};


} // namespace will::domain
