#pragma once

#include "acts/contemplation.h"
#include "identity/soul.h"
#include "properties/immanent.h"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Abode;
class Soul;
class Eternity;
class Spirit;


/// Heaven (Небо) — symbol of the invisible world.
/// Of the one World with Earth; Spirit alone reaches it via heaven().
/// Keeps Contemplations (who observes which Abode).
/// Immanent to itself. Creation brings forth Heaven. Heaven is in space and time;
/// Eternity is reached from here. Pointers address Soul bases of heap-stable Man.
class Heaven : private Immanent<Heaven> {
public:
	/// Whether Heaven knows this soul.
	bool knows(id::Soul soul_id) const;

	/// Living soul in the waking cosmos. Throws if unknown.
	const Soul& soul(id::Soul soul_id) const;

	/// Living souls whose Contemplation is this abode.
	std::vector<std::reference_wrapper<const Soul>> contemplating(const Abode& abode) const;

	/// Contemplation of a known soul. Throws if none.
	const Contemplation& contemplation(id::Soul soul_id) const;

protected:
	friend class Spirit;
	friend class Immanent<Heaven>;

	/// Bring forth Heaven (as World).
	explicit Heaven(Eternity& eternity);

	~Heaven();

	/// Eternity reached from Heaven (Spirit / Creation / World as Heaven).
	Eternity& eternity();
	const Eternity& eternity() const;

private:
	/// Present a soul owned by a heap-stable Man.
	void present(const Soul& soul);

	/// Record Contemplation of an abode for a known soul (Spirit).
	void contemplate(const Soul& soul, const Abode& abode);

	/// The one living Heaven. Throws if not yet brought forth / already destroyed.
	static Heaven& the();

	static Heaven* current_;

	Eternity& eternity_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, const Soul*> souls_;
	std::unordered_map<id::Soul, Contemplation> contemplations_;
};


} // namespace will::domain
