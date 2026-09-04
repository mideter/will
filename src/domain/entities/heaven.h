#pragma once

#include "entities/dead_vessel.h"
#include "entities/soul.h"
#include "entities/vessel.h"
#include "ids/soul.h"
#include "ports/eternity.h"

#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


/// Heaven (Небо) — runtime registry of souls. Speaks with Eternity.
class Heaven {
public:
    explicit Heaven(Eternity& eternity);

    std::optional<Soul> find_by_id(id::Soul id) const;

    /// Give a name, remember the soul in Eternity, and keep them in Heaven.
    std::pair<Soul, Vessel> remember_with_vessel(const DeadVessel& dead);

    std::vector<Vessel> load_vessels() const;

    void insert(Soul soul);

private:
    Eternity& eternity_;
    mutable std::mutex mutex_;
    std::unordered_map<id::Soul, Soul> souls_by_id_;
};


} // namespace will::domain
