#include "heaven.h"


namespace will::domain {


Heaven::Heaven(Eternity& eternity)
{
    for (God god : eternity.load_gods())
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


void Heaven::insert(God god)
{
    std::lock_guard lock(mutex_);
    gods_by_id_.insert_or_assign(god.id(), std::move(god));
}


} // namespace will::domain
