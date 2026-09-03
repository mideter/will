#pragma once

#include "entities/god.h"
#include "entities/vessel.h"
#include "ids/god.h"
#include "ports/eternity.h"

#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


/// Heaven (Небо) — runtime registry of gods. Speaks with Eternity.
class Heaven {
public:
    explicit Heaven(Eternity& eternity);

    std::optional<God> find_by_id(id::God id) const;

    /// Give a name, remember the god in Eternity, and keep them in Heaven.
    std::pair<God, Vessel> remember_with_vessel(std::string_view device_token);

    std::vector<Vessel> load_vessels() const;

    void insert(God god);

private:
    Eternity& eternity_;
    mutable std::mutex mutex_;
    std::unordered_map<id::God, God> gods_by_id_;
};


} // namespace will::domain
