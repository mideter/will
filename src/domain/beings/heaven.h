#pragma once

#include "identity/soul.h"

#include <mutex>
#include <unordered_map>


namespace will::domain {


class Soul;
class Eternity;
class Spirit;


/// Heaven (Небо) — of the one World; Spirit alone reaches it via heaven().
/// Creation brings forth Heaven.
/// Pointers address Soul bases of heap-stable Man (unique_ptr).
class Heaven {
public:
	/// Whether Heaven knows this soul.
	bool knows(id::Soul id) const;

	/// Living soul in the waking cosmos. Throws if unknown.
	const Soul& soul(id::Soul id) const;

protected:
	friend class Creation;
	friend class Spirit;

	/// Bring forth Heaven (Creation).
	explicit Heaven(Eternity& eternity);

	~Heaven();

	Heaven(const Heaven&) = delete;
	Heaven& operator=(const Heaven&) = delete;
	Heaven(Heaven&& other) noexcept;
	Heaven& operator=(Heaven&&) = delete;

	Eternity& eternity();
	const Eternity& eternity() const;

	/// Present a soul owned by a heap-stable Man.
	void present(const Soul& soul);

private:
	/// The one living Heaven. Throws if not yet brought forth / already destroyed.
	static Heaven& the();

	static Heaven* current_;

	Eternity& eternity_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, const Soul*> souls_;
};


} // namespace will::domain
