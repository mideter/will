#pragma once

#include "identity/abode.h"
#include "identity/man.h"
#include "identity/vessel.h"
#include "values/device_token.h"

#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


class Abode;
class Letter;
class Vessel;
class Temporality;
class World;


/// Earth (Земля) — of the one World; speaks with Temporality.
/// Dust reaches Temporality via friendship. Creation brings forth Earth.
/// Live registry of vessels and secondary abodes; global abode is the World itself.
class Earth {
public:
	/// The one living Earth. Throws if not yet brought forth / already destroyed.
	static Earth& the();

	/// Whether Earth knows this vessel.
	bool knows(id::Vessel id) const;

	/// Whether Earth holds this abode (World as global, others owned here).
	bool knows(id::Abode id) const;

	/// Living vessel by id. Throws if unknown.
	const Vessel& vessel(id::Vessel id) const;

	/// Living abode by id. Global abode is World via cast. Throws if unknown.
	Abode& abode(id::Abode id);
	const Abode& abode(id::Abode id) const;

protected:
	friend class Creation;
	friend class Dust;

	/// Bring forth Earth (Creation). Loads secondary abodes; World is the global abode.
	explicit Earth(Temporality& temporality);

	~Earth();

	Earth(const Earth&) = delete;
	Earth& operator=(const Earth&) = delete;
	Earth(Earth&& other) noexcept;
	Earth& operator=(Earth&&) = delete;

	/// Index a vessel owned by a heap-stable Man.
	void index(const Vessel& vessel);

	/// Own and index a secondary abode (from Temporality snapshot or birth).
	void index(Abode&& place);

	/// Resolve device token to vessel id for World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

	/// Record that a man dwells in an abode (through Temporality).
	void join_abode(id::Abode abode, id::Man man);

	/// Membership rows for restoring live abodes (through Temporality).
	std::vector<std::pair<id::Abode, id::Man>> abode_men() const;

private:
	static Earth* current_;

	Temporality& temporality_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Vessel, const Vessel*> vessels_;
	std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
	std::unordered_map<id::Abode, Abode*> abodes_;
	std::unordered_map<id::Abode, std::unique_ptr<Abode>> owned_abodes_;
};


} // namespace will::domain
