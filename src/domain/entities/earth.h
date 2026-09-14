#pragma once

#include "entities/letter.h"
#include "identity/abode.h"
#include "identity/man.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/word.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


class Abode;
class Vessel;
class Temporality;


/// Earth (Земля) — static pole of the one World; Dust is Earth.
/// Only World raises / clears the pole (owning shell). Other shells do not.
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

	/// Abode snapshots kept in Temporality (id + name).
	std::vector<Abode> remembered_abodes() const;

	/// Fix a word in time in an abode.
	void fix(id::Abode abode, id::Soul author, const Word& word) const;

	/// Letters kept in an abode, bounded by limit (history).
	std::vector<Letter> letters(id::Abode abode, std::uint32_t limit) const;

	/// Record abode membership in Temporality (idempotent).
	void join_abode(id::Abode abode, id::Man man) const;

	/// Membership rows for restoring live abodes.
	std::vector<std::pair<id::Abode, id::Man>> abode_men() const;

	bool operator==(const Earth&) const noexcept { return true; }

protected:
	/// Non-owning shell (Dust / Vessel). Does not touch static pole state.
	Earth() noexcept;

	/// Owning shell (World). Raises the static pole.
	explicit Earth(Temporality& temporality);

	~Earth();

	Earth(const Earth& other) noexcept;
	Earth& operator=(const Earth& other) noexcept;
	Earth(Earth&& other) noexcept;
	Earth& operator=(Earth&& other) noexcept;

	/// Index a vessel owned by a heap-stable Man.
	void index(const Vessel& vessel);

	/// Register a living abode (from Temporality snapshot or birth).
	void index(Abode place);

	/// Resolve device token to vessel id for World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

private:
	void clear_pole() noexcept;

	bool owns_pole_ = false;

	static Temporality* temporality_;
	static std::mutex mutex_;
	static std::unordered_map<id::Vessel, const Vessel*> vessels_;
	static std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
	static std::unordered_map<id::Abode, std::unique_ptr<Abode>> abodes_;
};


} // namespace will::domain
