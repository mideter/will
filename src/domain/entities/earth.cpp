#include "earth.h"


namespace will::domain {


Earth::Earth(Heaven& heaven)
    : heaven_(heaven)
{
    for (Vessel vessel : heaven_.load_vessels())
        insert(std::move(vessel));
}


std::optional<God> Earth::receive(const std::string_view device_token_raw)
{
    if (const std::optional<Vessel> vessel = find_by_token(device_token_raw))
        return heaven_.find_by_id(vessel->god_id());

    const std::optional<DeviceToken> token = DeviceToken::parse(device_token_raw);
    if (!token)
        return std::nullopt;

    auto [god, vessel] = heaven_.remember_with_vessel(token->text());
    insert(std::move(vessel));
    return god;
}


std::optional<Vessel> Earth::find_by_token(const std::string_view device_token) const
{
    const auto token = DeviceToken::parse(device_token);
    if (!token)
        return std::nullopt;

    std::lock_guard lock(mutex_);

    const auto token_it = id_by_token_.find(*token);
    if (token_it == id_by_token_.end())
        return std::nullopt;

    const auto it = vessels_by_id_.find(token_it->second);
    if (it == vessels_by_id_.end())
        return std::nullopt;

    return it->second;
}


std::optional<id::God> Earth::god_id_for_token(const std::string_view device_token) const
{
    if (const std::optional<Vessel> vessel = find_by_token(device_token))
        return vessel->god_id();
    return std::nullopt;
}


void Earth::insert(Vessel vessel)
{
    std::lock_guard lock(mutex_);
    id_by_token_.insert_or_assign(vessel.token(), vessel.id());
    vessels_by_id_.insert_or_assign(vessel.id(), std::move(vessel));
}


} // namespace will::domain
