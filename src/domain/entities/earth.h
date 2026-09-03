#pragma once

#include "entities/vessel.h"
#include "ids/god.h"
#include "ids/vessel.h"
#include "ports/eternity.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>


namespace will::domain {


/// Earth (Земля) — runtime registry of vessels. Persistence goes through Eternity.
class Earth {
public:
    explicit Earth(Eternity& eternity);

    std::optional<Vessel> find_by_token(std::string_view device_token) const;
    std::optional<id::God> god_id_for_token(std::string_view device_token) const;

    void insert(Vessel vessel);

private:
    mutable std::mutex mutex_;
    std::unordered_map<id::Vessel, Vessel> vessels_by_id_;
    std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
