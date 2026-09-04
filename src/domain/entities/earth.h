#pragma once

#include "entities/heaven.h"
#include "entities/man.h"
#include "entities/vessel.h"
#include "ids/man.h"
#include "ids/soul.h"
#include "ids/vessel.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>


namespace will::domain {


/// Earth (Земля) — runtime registry of vessels and men. Looks to Heaven; does not know Eternity.
class Earth {
public:
    explicit Earth(Heaven& heaven);

    std::optional<Vessel> find_vessel_by_token(const DeviceToken& token) const;
    std::optional<Man> find_man_by_token(const DeviceToken& token) const;
    std::optional<id::Soul> soul_id_for_token(const DeviceToken& token) const;

    void insert(Vessel vessel);
    void insert(Man man);

private:
    Heaven& heaven_;
    mutable std::mutex mutex_;
    std::unordered_map<id::Vessel, Vessel> vessels_by_id_;
    std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
    std::unordered_map<id::Man, Man> men_by_id_;
    std::unordered_map<id::Vessel, id::Man> man_id_by_vessel_;
};


} // namespace will::domain
