#include "clienthub.h"

#include <algorithm>

#include "client.h"


namespace will {


void ClientHub::add(std::shared_ptr<Client> client)
{
	if (!client)
		return;
	std::lock_guard lock(mutex_);
	clients_.push_back(std::move(client));
}


void ClientHub::remove(const Client* identity)
{
	if (!identity)
		return;
	std::lock_guard lock(mutex_);
	std::erase_if(clients_, [identity](const std::shared_ptr<Client>& c) {
		return c.get() == identity;
	});
}


std::vector<std::shared_ptr<Client>> ClientHub::snapshot() const
{
	std::lock_guard lock(mutex_);
	return clients_;
}


void ClientHub::reset()
{
	std::lock_guard lock(mutex_);
	clients_.clear();
}


std::size_t ClientHub::count() const noexcept
{
	std::lock_guard lock(mutex_);
	return clients_.size();
}


} // namespace will
