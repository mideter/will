#include "heaven.h"

#include "values/god_name.h"


namespace will::domain {


Heaven::Heaven(Eternity& eternity)
    : eternity_(eternity)
{
    for (God god : eternity_.load_gods())
        insert(std::move(god));
}


std::optional<God> Heaven::find_by_id(const id::God id) const
{
    std::lock_guard lock(mutex_);

    const auto it = gods_by_id_.find(id);
    if (it == gods_by_id_.end())
        return std::nullopt;

    return it->second;
}


std::pair<God, Vessel> Heaven::remember_with_vessel(const DeadVessel& dead)
{
    const GodName name = GodName::generate();
    auto [god, vessel] = eternity_.insert_god_with_vessel(dead.text(), name);
    insert(god);
    return {god, std::move(vessel)};
}


std::vector<Vessel> Heaven::load_vessels() const
{
    return eternity_.load_vessels();
}


void Heaven::insert(God god)
{
    std::lock_guard lock(mutex_);
    gods_by_id_.insert_or_assign(god.id(), std::move(god));
}


} // namespace will::domain
