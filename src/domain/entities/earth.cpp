#include "earth.h"


namespace will::domain {


Earth::Earth(Heaven& heaven)
    : heaven_(heaven)
{
    for (Vessel vessel : heaven_.load_vessels())
        insert(std::move(vessel));
    for (Man man : heaven_.load_men())
        insert(std::move(man));
}


std::optional<Vessel> Earth::find_vessel_by_token(const DeviceToken& token) const
{
    std::lock_guard lock(mutex_);

    const auto token_it = id_by_token_.find(token);
    if (token_it == id_by_token_.end())
        return std::nullopt;

    const auto it = vessels_by_id_.find(token_it->second);
    if (it == vessels_by_id_.end())
        return std::nullopt;

    return it->second;
}


std::optional<Man> Earth::find_man_by_token(const DeviceToken& token) const
{
    std::lock_guard lock(mutex_);

    const auto token_it = id_by_token_.find(token);
    if (token_it == id_by_token_.end())
        return std::nullopt;

    const auto man_it = man_id_by_vessel_.find(token_it->second);
    if (man_it == man_id_by_vessel_.end())
        return std::nullopt;

    const auto it = men_by_id_.find(man_it->second);
    if (it == men_by_id_.end())
        return std::nullopt;

    return it->second;
}


std::optional<id::Soul> Earth::soul_id_for_token(const DeviceToken& token) const
{
    if (const std::optional<Man> man = find_man_by_token(token))
        return man->soul_id();
    return std::nullopt;
}


void Earth::insert(Vessel vessel)
{
    std::lock_guard lock(mutex_);
    id_by_token_.insert_or_assign(vessel.token(), vessel.id());
    vessels_by_id_.insert_or_assign(vessel.id(), std::move(vessel));
}


void Earth::insert(Man man)
{
    std::lock_guard lock(mutex_);
    man_id_by_vessel_.insert_or_assign(man.vessel_id(), man.id());
    men_by_id_.insert_or_assign(man.id(), std::move(man));
}


} // namespace will::domain
