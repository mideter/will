#pragma once

#include "entities/man.h"
#include "entities/soul.h"
#include "entities/vessel.h"
#include "ids/soul.h"
#include "ports/eternity.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>


namespace will::domain {


/// Heaven (Небо) — runtime registry of souls. Speaks with Eternity.
class Heaven {
public:
    explicit Heaven(Eternity& eternity);

    std::optional<Soul> find_by_id(id::Soul id) const;

    /// Give a name, remember soul/vessel/man in Eternity, and keep the soul in Heaven.
    ManBirth remember_man(const DeviceToken& token);

    std::vector<Vessel> load_vessels() const;
    std::vector<Man> load_men() const;

    void insert(Soul soul);

private:
    Eternity& eternity_;
    mutable std::mutex mutex_;
    std::unordered_map<id::Soul, Soul> souls_by_id_;
};


} // namespace will::domain
