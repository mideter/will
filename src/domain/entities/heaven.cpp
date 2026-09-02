#include "heaven.h"


namespace will::domain {


Heaven::Heaven(HeavenStore& store)
    : store_(store)
{
    for (God god : store_.load_all())
        insert(std::move(god));
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

    const auto token_it = id_by_token_.find(*token);
    if (token_it == id_by_token_.end())
        return std::nullopt;

    const auto it = gods_by_id_.find(token_it->second);
    if (it == gods_by_id_.end())
        return std::nullopt;

    return it->second;
}


God Heaven::create_god(const std::string_view device_token, const GodName name)
{
    God god = store_.insert(device_token, name);

    std::lock_guard lock(mutex_);
    id_by_token_.insert_or_assign(god.device_token(), god.id());
    gods_by_id_.insert_or_assign(god.id(), god);
    return god;
}


void Heaven::insert(God god)
{
    std::lock_guard lock(mutex_);
    id_by_token_.insert_or_assign(god.device_token(), god.id());
    gods_by_id_.insert_or_assign(god.id(), std::move(god));
}


} // namespace will::domain
