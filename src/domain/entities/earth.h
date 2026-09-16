#pragma once

#include "identity/abode.h"
#include "identity/man.h"
#include "identity/vessel.h"
#include "values/abode_name.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


class Letter;
class Vessel;
class Temporality;


/// Earth (Земля) — of the one World; speaks with Temporality.
/// Dust reaches Temporality via friendship. Creation brings forth Earth.
/// Live registry of vessels; abodes live with men in the World.
class Earth {
public:
	/// Whether Earth knows this vessel.
	bool knows(id::Vessel id) const;

	/// Living vessel by id. Throws if unknown.
	const Vessel& vessel(id::Vessel id) const;

protected:
	friend class Creation;
	friend class Dust;

	/// Bring forth Earth (Creation).
	explicit Earth(Temporality& temporality);

	~Earth();

	Earth(const Earth&) = delete;
	Earth& operator=(const Earth&) = delete;
	Earth(Earth&& other) noexcept;
	Earth& operator=(Earth&&) = delete;

	/// Present a vessel owned by a heap-stable Man.
	void present(const Vessel& vessel);

	/// Resolve device token to vessel id for World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

	/// Keep an abode in Temporality (idempotent by id).
	void keep(id::Abode id, AbodeName name);

	/// Record that a man dwells in an abode (through Temporality).
	void join_abode(id::Abode abode, id::Man man);

	/// Membership rows for restoring live abodes (through Temporality).
	std::vector<std::pair<id::Abode, id::Man>> abode_men() const;

private:
	/// The one living Earth. Throws if not yet brought forth / already destroyed.
	static Earth& the();

	static Earth* current_;

	Temporality& temporality_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Vessel, const Vessel*> vessels_;
	std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
