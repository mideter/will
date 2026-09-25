#pragma once

#include "beings/man.h"
#include "beings/place.h"
#include "identity/abode.h"
#include "values/abode_name.h"

#include <memory>
#include <mutex>
#include <unordered_set>


namespace will::domain {


/// Abode (Обитель) — the host's place: observers may dwell here.
/// Place id from Space::point; host is the Witness who keeps it. Rooms come later.
/// A living Witness observes one abode. Movable: Spatiality may hand
/// id+name snapshots; dwellers fill in the World.
class Abode : public Place {
public:
	Abode(id::Abode id, AbodeName name);
	Abode(Abode&& other) noexcept;
	Abode& operator=(Abode&& other) noexcept;
	Abode(const Abode&) = delete;
	Abode& operator=(const Abode&) = delete;

	/// Strongly typed abode id (same value as Place::id()).
	id::Abode abode_id() const noexcept { return id::Abode{id().value()}; }

	const AbodeName& name() const noexcept { return name_; }

	/// Admit a man as dweller (participation).
	void admit(const Man& man);

	/// Whether this man dwells here (participant).
	bool dwells(const Man& man) const;

private:
	AbodeName name_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_set<const Man*> dwellers_;
};


} // namespace will::domain
