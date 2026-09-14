#pragma once

#include "identity/soul.h"

#include <mutex>
#include <unordered_map>


namespace will::domain {


class Soul;
class Eternity;


/// Heaven (Небо) — static pole of the one World; Spirit is Heaven.
/// Creation raises the pole; Creation lowers it. Other shells do not.
/// Pointers address Soul bases of heap-stable Man (unique_ptr).
class Heaven {
public:
	/// Whether Heaven knows this soul.
	bool knows(id::Soul id) const;

	/// Living soul in the waking cosmos. Throws if unknown.
	const Soul& soul(id::Soul id) const;

	bool operator==(const Heaven&) const noexcept { return true; }

protected:
	friend class Creation;

	/// Non-owning shell (Spirit / Soul). Does not touch static pole state.
	Heaven() noexcept;

	/// Raise the static pole (Creation).
	explicit Heaven(Eternity& eternity);

	~Heaven() = default;

	Heaven(const Heaven&) noexcept = default;
	Heaven& operator=(const Heaven&) noexcept = default;
	Heaven(Heaven&&) noexcept = default;
	Heaven& operator=(Heaven&&) noexcept = default;

	/// Eternity of the raised pole. Throws if Heaven is not raised.
	Eternity& eternity();
	const Eternity& eternity() const;

	/// Index a soul owned by a heap-stable Man.
	void index(const Soul& soul);

	/// Lower the static pole (Creation dtor only).
	void lower() noexcept;

private:
	static Eternity* eternity_;
	static std::mutex mutex_;
	static std::unordered_map<id::Soul, const Soul*> souls_;
};


} // namespace will::domain
