#pragma once

#include "identity/abode.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/word.h"

#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>


namespace will::domain {


class Abode;
class Letter;
class Vessel;
class Temporality;


/// Earth (Земля) — static pole of the one World; Dust is Earth.
/// Only World raises the pole (ctor) and lowers it (dtor). Other shells do not.
/// Public API: live registry of vessels and abodes. Temporality is protected.
class Earth {
public:
	/// Whether Earth knows this vessel.
	bool knows(id::Vessel id) const;

	/// Whether Earth holds this abode.
	bool knows(id::Abode id) const;

	/// Living vessel by id. Throws if unknown.
	const Vessel& vessel(id::Vessel id) const;

	/// Living abode by id. Throws if unknown.
	Abode& abode(id::Abode id);
	const Abode& abode(id::Abode id) const;

	/// Global abode (id 1) — default until birth/choice of others.
	Abode& abode() { return abode(id::Abode::global()); }
	const Abode& abode() const { return abode(id::Abode::global()); }

	bool operator==(const Earth&) const noexcept { return true; }

protected:
	/// Non-owning shell (Dust / Vessel). Does not touch static pole state.
	Earth() noexcept;

	/// Raise the static pole (World).
	explicit Earth(Temporality& temporality);

	~Earth() = default;

	Earth(const Earth&) noexcept = default;
	Earth& operator=(const Earth&) noexcept = default;
	Earth(Earth&&) noexcept = default;
	Earth& operator=(Earth&&) noexcept = default;

	/// Temporality of the raised pole. Throws if Earth is not raised.
	Temporality& temporality();
	const Temporality& temporality() const;

	/// Fix a word in time (Dust / Witness).
	void fix(id::Abode abode, id::Soul author, const Word& word) const;

	/// Index a vessel owned by a heap-stable Man.
	void index(const Vessel& vessel);

	/// Register a living abode (from Temporality snapshot or birth).
	void index(Abode place);

	/// Resolve device token to vessel id for World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

	/// Lower the static pole (World dtor only).
	void lower() noexcept;

private:
	static Temporality* temporality_;
	static std::mutex mutex_;
	static std::unordered_map<id::Vessel, const Vessel*> vessels_;
	static std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
	static std::unordered_map<id::Abode, std::unique_ptr<Abode>> abodes_;
};


} // namespace will::domain
