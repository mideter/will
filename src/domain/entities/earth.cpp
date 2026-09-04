#include "earth.h"


namespace will::domain {


Earth::Earth(Heaven& heaven)
    : heaven_(heaven)
{
    for (Vessel vessel : heaven_.load_vessels())
        insert(std::move(vessel));
}


std::optional<Vessel> Earth::find_by_dead(const DeadVessel& dead) const
{
    std::lock_guard lock(mutex_);

    const auto token_it = id_by_dead_.find(dead);
    if (token_it == id_by_dead_.end())
        return std::nullopt;

    const auto it = vessels_by_id_.find(token_it->second);
    if (it == vessels_by_id_.end())
        return std::nullopt;

    return it->second;
}


std::optional<id::God> Earth::god_id_for_dead(const DeadVessel& dead) const
{
    if (const std::optional<Vessel> vessel = find_by_dead(dead))
        return vessel->god_id();
    return std::nullopt;
}


void Earth::insert(Vessel vessel)
{
    std::lock_guard lock(mutex_);
    id_by_dead_.insert_or_assign(vessel.dead(), vessel.id());
    vessels_by_id_.insert_or_assign(vessel.id(), std::move(vessel));
}


} // namespace will::domain
