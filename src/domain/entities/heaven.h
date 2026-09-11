#pragma once

#include "identity/soul.h"

#include <mutex>
#include <unordered_map>
#include <utility>


namespace will::domain {


class Soul;
class Eternity;


/// Heaven (Небо) — static pole of the one World; Spirit is Heaven.
/// Only World raises / clears the pole (owning shell). Other shells do not.
/// Pointers address Soul bases of heap-stable Man (unique_ptr).
class Heaven {
public:
	/// Whether Heaven knows this soul.
	bool knows(id::Soul id) const;

	/// Living soul in the waking cosmos. Throws if unknown.
	const Soul& soul(id::Soul id) const;

	bool operator==(const Heaven&) const noexcept { return true; }

protected:
	/// Non-owning shell (Spirit / Soul). Does not touch static pole state.
	Heaven() noexcept;

	/// Owning shell (World). Raises the static pole.
	explicit Heaven(Eternity& eternity);

	~Heaven();

	Heaven(const Heaven& other) noexcept;
	Heaven& operator=(const Heaven& other) noexcept;
	Heaven(Heaven&& other) noexcept;
	Heaven& operator=(Heaven&& other) noexcept;

	/// Eternity of the raised pole. Throws if Heaven is not raised.
	Eternity& eternity();
	const Eternity& eternity() const;

	/// Index a soul owned by a heap-stable Man.
	void index(const Soul& soul);

private:
	void clear_pole() noexcept;

	bool owns_pole_ = false;

	static Eternity* eternity_;
	static std::mutex mutex_;
	static std::unordered_map<id::Soul, const Soul*> souls_by_id_;
};


} // namespace will::domain
