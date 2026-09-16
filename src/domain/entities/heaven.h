#pragma once

#include "identity/soul.h"

#include <mutex>
#include <unordered_map>


namespace will::domain {


class Soul;
class Eternity;


/// Heaven (Небо) — of the one World; Spirit knows Heaven via the().
/// Creation brings forth Heaven.
/// Pointers address Soul bases of heap-stable Man (unique_ptr).
class Heaven {
public:
	/// The one living Heaven. Throws if not yet brought forth / already destroyed.
	static Heaven& the();

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

	/// Index a soul owned by a heap-stable Man.
	void index(const Soul& soul);

private:
	static Heaven* current_;

	Eternity& eternity_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, const Soul*> souls_;
};


} // namespace will::domain
