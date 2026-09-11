#pragma once

#include "entities/letter.h"
#include "identity/abode.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/word.h"

#include <cstdint>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>


namespace will::domain {


class Vessel;
class Temporality;


/// Earth (Земля) — static pole of the one World; Dust is Earth.
/// Only World raises / clears the pole (owning shell). Other shells do not.
class Earth {
public:
	/// Whether Earth knows this vessel.
	bool knows(id::Vessel id) const;

	/// Living vessel by id. Throws if unknown.
	const Vessel& vessel(id::Vessel id) const;

	/// Fix a word in time in an abode.
	void fix(id::Abode abode, id::Soul author, const Word& word) const;

	/// Letters kept in an abode, bounded by limit (history).
	std::vector<Letter> letters(id::Abode abode, std::uint32_t limit) const;

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

	/// Resolve device token to vessel id for World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

private:
	void clear_pole() noexcept;

	bool owns_pole_ = false;

	static Temporality* temporality_;
	static std::mutex mutex_;
	static std::unordered_map<id::Vessel, const Vessel*> vessels_by_id_;
	static std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
