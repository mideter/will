#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>


namespace will {


class Client;


/**
 * Thread-safe registry of connected {@link Client} instances.
 *
 * Invariants:
 * - {@code clients_} is read or modified only while {@code mutex_} is held.
 * - Every stored {@code shared_ptr<Client>} is non-null (null arguments to {@link add} are ignored).
 */
class ClientHub {
public:
	ClientHub() = default;
	~ClientHub() = default;

	ClientHub(const ClientHub&) = delete;
	ClientHub& operator=(const ClientHub&) = delete;
	ClientHub(ClientHub&&) = delete;
	ClientHub& operator=(ClientHub&&) = delete;

	/** Registers a live client; no-op if {@code client} is null. */
	void add(std::shared_ptr<Client> client);

	/** Removes the entry whose raw pointer matches {@code identity}; no-op if {@code identity} is null. */
	void remove(const Client* identity);

	/** Consistent point-in-time copy of current members (each non-null). */
	std::vector<std::shared_ptr<Client>> snapshot() const;

	/** Clears all registrations under the mutex. */
	void reset();

	/** Number of registered clients (O(1) size query under lock). */
	std::size_t count() const noexcept;

private:
	mutable std::mutex mutex_;
	std::vector<std::shared_ptr<Client>> clients_;
};


} // namespace will
