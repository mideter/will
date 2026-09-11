#pragma once

#include "entities/letter.h"
#include "entities/vessel.h"
#include "identity/abode.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "ports/temporality.h"
#include "values/device_token.h"
#include "values/word.h"

#include <cstdint>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>


namespace will::domain {


/// Earth (Земля) — static pole of the one World: what changes and is fixed in time.
/// Speaks with Temporality (as Heaven speaks with Eternity).
/// Also the living index of vessels. World is Earth and raises / clears this state in its life.
class Earth {
public:
	/// Whether Earth knows this vessel.
	bool knows(id::Vessel id) const;

	/// Living vessel by id. Throws if unknown.
	const Vessel& vessel(id::Vessel id) const;

	/// Fix a word in time in an abode.
	void fix(id::Abode abode, id::Soul author, const Word& word);

	/// Letters kept in an abode, bounded by limit (history).
	std::vector<Letter> letters(id::Abode abode, std::uint32_t limit) const;

protected:
	explicit Earth(Temporality& temporality);
	~Earth();

	Earth(const Earth&) = delete;
	Earth& operator=(const Earth&) = delete;

	/// Index a vessel owned by a heap-stable Man.
	void index(const Vessel& vessel);

	/// Resolve device token to vessel id for World::welcome. Empty if unknown.
	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

private:
	static Temporality* temporality_;
	static std::mutex mutex_;
	static std::unordered_map<id::Vessel, const Vessel*> vessels_by_id_;
	static std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
