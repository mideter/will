#pragma once

#include "beings/immanent.h"
#include "identity/abode.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/abode_name.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


class Vessel;
class Temporality;
class Spatiality;


/// Earth (Земля) — of the one World; speaks with Temporality and Spatiality.
/// Dust reaches both via friendship. Creation brings forth Earth.
/// Earth is in space and time. Live registry of vessels; abodes live with men;
/// places are known to Space; spatial material to Spatiality.
class Earth : private Immanent {
public:
	/// Whether Earth knows this vessel.
	bool knows(id::Vessel id) const;

	/// Living vessel by id. Throws if unknown.
	const Vessel& vessel(id::Vessel id) const;

protected:
	friend class Dust;

	/// Bring forth Earth (as World).
	Earth(Temporality& temporality, Spatiality& spatiality);

	~Earth();

	/// Present a vessel owned by a heap-stable Man.
	void present(const Vessel& vessel);

	/// Resolve device token to vessel id for World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

	/// Keep an abode in Spatiality (idempotent by id).
	void keep(id::Abode id, AbodeName name);

	/// Record that a soul dwells in an abode (through Spatiality).
	void join_abode(id::Abode abode, id::Soul soul);

	Temporality& temporality() noexcept { return temporality_; }
	const Temporality& temporality() const noexcept { return temporality_; }

	Spatiality& spatiality() noexcept { return spatiality_; }
	const Spatiality& spatiality() const noexcept { return spatiality_; }

private:
	/// The one living Earth. Throws if not yet brought forth / already destroyed.
	static Earth& the();

	static Earth* current_;

	Temporality& temporality_;
	Spatiality& spatiality_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Vessel, const Vessel*> vessels_;
	std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
