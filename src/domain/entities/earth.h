#pragma once

#include "entities/god.h"
#include "entities/heaven.h"
#include "entities/vessel.h"
#include "ids/god.h"
#include "ids/vessel.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>


namespace will::domain {


/// Earth (Земля) — runtime registry of vessels. Looks to Heaven; does not know Eternity.
class Earth {
public:
    explicit Earth(Heaven& heaven);

    /// Find an existing vessel's god, or receive a new vessel through Heaven.
    std::optional<God> receive(std::string_view device_token_raw);

    std::optional<Vessel> find_by_token(std::string_view device_token) const;
    std::optional<id::God> god_id_for_token(std::string_view device_token) const;

    void insert(Vessel vessel);

private:
    Heaven& heaven_;
    mutable std::mutex mutex_;
    std::unordered_map<id::Vessel, Vessel> vessels_by_id_;
    std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
