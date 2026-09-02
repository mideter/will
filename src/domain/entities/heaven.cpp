#include "heaven.h"


namespace will::domain {


Heaven::Heaven(Eternity& eternity)
    : eternity_(eternity)
{
    for (God god : eternity_.load_gods())
        insert(std::move(god));
    for (Vessel vessel : eternity_.load_vessels())
        insert(std::move(vessel));
}


std::optional<God> Heaven::find_by_id(const GodId id) const
{
    std::lock_guard lock(mutex_);

    const auto it = gods_by_id_.find(id);
    if (it == gods_by_id_.end())
        return std::nullopt;

    return it->second;
}


std::optional<God> Heaven::find_by_device_token(const std::string_view device_token) const
{
    const auto token = DeviceToken::parse(device_token);
    if (!token)
        return std::nullopt;

    std::lock_guard lock(mutex_);

    const auto token_it = god_id_by_token_.find(*token);
    if (token_it == god_id_by_token_.end())
        return std::nullopt;

    const auto it = gods_by_id_.find(token_it->second);
    if (it == gods_by_id_.end())
        return std::nullopt;

    return it->second;
}


God Heaven::create_god(const std::string_view device_token, const GodName name)
{
    auto [god, vessel] = eternity_.insert_god_with_vessel(device_token, name);

    std::lock_guard lock(mutex_);
    gods_by_id_.insert_or_assign(god.id(), god);
    god_id_by_token_.insert_or_assign(vessel.token(), god.id());
    return god;
}


void Heaven::insert(God god)
{
    std::lock_guard lock(mutex_);
    gods_by_id_.insert_or_assign(god.id(), std::move(god));
}


void Heaven::insert(Vessel vessel)
{
    std::lock_guard lock(mutex_);
    god_id_by_token_.insert_or_assign(vessel.token(), vessel.god_id());
}


void Heaven::insert_god_with_vessel(God god, Vessel vessel)
{
    insert(std::move(god));
    insert(std::move(vessel));
}


} // namespace will::domain
