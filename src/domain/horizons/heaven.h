#pragma once

#include "immanents/contemplation.h"
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


/// Heaven (Небо) — symbol of the invisible world.
/// Keeps Contemplations and pending Supplications — the living relations of souls. Heaven is in space and time; Eternity is reached from here.
/// Pointers address Soul bases of heap-stable Man.
class Heaven : private Immanent<Heaven> {
public:
	bool knows(id::Soul soul_id) const;

	/// Throws if unknown.
	const Soul& soul(id::Soul soul_id) const;

	std::vector<std::reference_wrapper<const Soul>> contemplating(const Place& place) const;

	/// Supplications awaiting this soul's answer, oldest first.
	/// Whoever holds one keeps it from fading while he uses it.
	std::vector<std::shared_ptr<const Supplication>> supplications(id::Soul addressee) const;

	/// The gaze of this soul; null while the soul contemplates nothing (asleep).
	/// Whoever holds it keeps it from fading while he uses it.
	std::shared_ptr<const Contemplation> contemplation(id::Soul soul_id) const;

protected:
	friend class Spirit;
	friend class Immanent<Heaven>;

	/// As World.
	explicit Heaven(Eternity& eternity);

	~Heaven();

	Eternity& eternity();
	const Eternity& eternity() const;

private:
	/// Soul owned by a heap-stable Man.
	void present(const Soul& soul);

	void contemplate(const Soul& soul, const Place& place);

	/// The gaze of this soul ends; nothing happens if it contemplates nothing.
	void cease(const Soul& soul);

	/// Keep a supplication heard by its addressee until he answers it.
	void keep(std::shared_ptr<const Supplication> supplication);

	/// Let an answered supplication go.
	void release(const Supplication& supplication);

	/// Throws if not yet brought forth / already destroyed.
	static Heaven& the();

	static Heaven* current_;

	Eternity& eternity_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, const Soul*> souls_;
	std::unordered_map<id::Soul, std::shared_ptr<const Contemplation>> contemplations_;
	std::vector<std::shared_ptr<const Supplication>> supplications_;
};


} // namespace will::domain
