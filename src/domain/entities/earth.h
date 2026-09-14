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


/// Earth (Земля) — of the one World.
/// Creation brings forth Earth and rolls it up. Other shells do not.
/// Live registry of vessels and abodes (World as global, others owned here).
class Earth {
public:
	/// Whether Earth knows this vessel.
	bool knows(id::Vessel id) const;

	/// Whether Earth holds this abode (including the World).
	bool knows(id::Abode id) const;

	/// Living vessel by id. Throws if unknown.
	const Vessel& vessel(id::Vessel id) const;

	/// Living abode by id. Throws if unknown.
	Abode& abode(id::Abode id);
	const Abode& abode(id::Abode id) const;

	bool operator==(const Earth&) const noexcept { return true; }

protected:
	friend class Creation;

	/// Non-owning shell (Dust / Vessel). Does not bring forth or roll Earth.
	Earth() noexcept;

	/// Bring forth Earth (Creation). Loads secondary abodes; World is indexed later.
	explicit Earth(Temporality& temporality);

	~Earth() = default;

	Earth(const Earth&) noexcept = default;
	Earth& operator=(const Earth&) noexcept = default;
	Earth(Earth&&) noexcept = default;
	Earth& operator=(Earth&&) noexcept = default;

	/// Temporality once Earth is brought forth. Throws otherwise.
	Temporality& temporality();
	const Temporality& temporality() const;

	/// Fix a word in time (Dust / Witness).
	void fix(id::Abode abode, id::Soul author, const Word& word) const;

	/// Index a vessel owned by a heap-stable Man.
	void index(const Vessel& vessel);

	/// Non-owning index (Creation registers the World-abode).
	void index(Abode& place);

	/// Own and index a secondary abode (from Temporality snapshot or birth).
	void index(Abode&& place);

	/// Resolve device token to vessel id for World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

	/// Roll up Earth (Creation dtor only).
	void roll() noexcept;

private:
	static Temporality* temporality_;
	static std::mutex mutex_;
	static std::unordered_map<id::Vessel, const Vessel*> vessels_;
	static std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
	static std::unordered_map<id::Abode, Abode*> abodes_;
	static std::unordered_map<id::Abode, std::unique_ptr<Abode>> owned_abodes_;
};


} // namespace will::domain
