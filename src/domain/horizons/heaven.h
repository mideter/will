#pragma once

#include "relations/contemplation.h"
#include "identity/soul.h"
#include "properties/immanent.h"

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Abode;
class Place;
class Soul;
class Eternity;
class Spirit;
class Supplication;


/// Heaven (Небо) — the invisible world: the souls, their gazes and the supplications
/// awaiting answer; Eternity is reached from here.
class Heaven : private Immanent<Heaven> {
public:
	bool knows(id::Soul soul_id) const;

	const Soul& soul(id::Soul soul_id) const;

	/// On the place itself, or on a room reflecting it.
	std::vector<std::reference_wrapper<const Soul>> contemplating(const Place& place) const;

	/// Oldest first. Whoever holds one keeps it from fading while he uses it.
	std::vector<std::shared_ptr<const Supplication>> supplications(id::Soul addressee) const;

	/// Null while asleep. Whoever holds it keeps it from fading while he uses it.
	std::shared_ptr<const Contemplation> contemplation(id::Soul soul_id) const;

protected:
	friend class Spirit;
	friend class Immanent<Heaven>;

	explicit Heaven(Eternity& eternity);

	~Heaven();

	Eternity& eternity();
	const Eternity& eternity() const;

private:
	void present(const Soul& soul);

	void contemplate(const Soul& soul, const Place& place);

	void cease(const Soul& soul);

	void keep(std::shared_ptr<const Supplication> supplication);

	void release(const Supplication& supplication);

	static Heaven& the();

	static Heaven* current_;

	Eternity& eternity_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, const Soul*> souls_;
	std::unordered_map<id::Soul, std::shared_ptr<const Contemplation>> contemplations_;
	std::vector<std::shared_ptr<const Supplication>> supplications_;
};


} // namespace will::domain
