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
class Soul;
class Eternity;
class Spirit;


/// Heaven (Небо) — symbol of the invisible world.
/// Keeps Contemplations. Heaven is in space and time; Eternity is reached from here.
/// Pointers address Soul bases of heap-stable Man.
class Heaven : private Immanent<Heaven> {
public:
	bool knows(id::Soul soul_id) const;

	/// Throws if unknown.
	const Soul& soul(id::Soul soul_id) const;

	std::vector<std::reference_wrapper<const Soul>> contemplating(const Abode& abode) const;

	/// The gazes resting now upon this abode.
	std::vector<std::shared_ptr<const Contemplation>> gazes(const Abode& abode) const;

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

	void contemplate(const Soul& soul, const Abode& abode);

	/// The gaze of this soul ends; nothing happens if it contemplates nothing.
	void cease(const Soul& soul);

	/// Throws if not yet brought forth / already destroyed.
	static Heaven& the();

	static Heaven* current_;

	Eternity& eternity_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, const Soul*> souls_;
	std::unordered_map<id::Soul, std::shared_ptr<const Contemplation>> contemplations_;
};


} // namespace will::domain
