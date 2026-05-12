#include "clienthub.h"

#include <algorithm>
#include <iostream>

#include "client.h"


namespace will {


void ClientHub::add(std::shared_ptr<Client> client)
{
	if (!client)
		return;

	std::lock_guard lock(mutex_);
	std::cout << "Client " << client->address() << " connected" << std::endl;
	
	clients_.push_back(std::move(client));
}


void ClientHub::remove(const Client* identity)
{
	if (!identity)
		return;

	std::lock_guard lock(mutex_);

	auto predicate = [identity](const std::shared_ptr<Client>& c) { return c.get() == identity; };
	const auto it = std::ranges::find_if(clients_, predicate);
	
	if (it == clients_.end())
		return;
	
	std::cout << "Client " << (*it)->address() << " disconnected" << std::endl;
	clients_.erase(it);
}


std::vector<std::shared_ptr<Client>> ClientHub::snapshot() const
{
	std::lock_guard lock(mutex_);
	return clients_;
}


void ClientHub::reset()
{
	std::lock_guard lock(mutex_);

	for (const std::shared_ptr<Client>& c : clients_)
		std::cout << "Client " << c->address() << " disconnected" << std::endl;
	
	clients_.clear();
}


std::size_t ClientHub::count() const noexcept
{
	std::lock_guard lock(mutex_);
	return clients_.size();
}


} // namespace will
