#pragma once

#include "properties/immanent.h"
#include "identity/soul.h"

#include <mutex>
#include <unordered_map>


namespace will::domain {


class Soul;
class Eternity;
class Spirit;


/// Heaven (Небо) — of the one World; Spirit alone reaches it via heaven().
/// Immanent to itself. Creation brings forth Heaven. Heaven is in space and time;
/// Eternity is reached from here. Pointers address Soul bases of heap-stable Man.
class Heaven : private Immanent<Heaven> {
public:
	/// Whether Heaven knows this soul.
	bool knows(id::Soul soul_id) const;

	/// Living soul in the waking cosmos. Throws if unknown.
	const Soul& soul(id::Soul soul_id) const;

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

	/// The one living Heaven. Throws if not yet brought forth / already destroyed.
	static Heaven& the();

	static Heaven* current_;

	Eternity& eternity_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, const Soul*> souls_;
};


} // namespace will::domain
