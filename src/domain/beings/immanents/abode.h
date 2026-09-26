#pragma once

#include "acts/abiding.h"
#include "beings/immanents/place.h"
#include "identity/abode.h"
#include "values/abode_name.h"

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <vector>


namespace will::domain {


class Man;
class Witness;


/// Abode (Обитель) — the host's place: men may dwell; Witnesses may observe.
/// Place id from Space::point; host is the Witness who keeps it. Rooms come later.
/// A living Witness observes one abode. Born from Abiding material or at first welcome.
class Abode : public Place {
public:
	Abode(id::Abode id, AbodeName name);
	explicit Abode(Abiding abiding);

	/// Strongly typed abode id (same value as Place::id()).
	id::Abode abode_id() const noexcept { return id::Abode{id()}; }

	const AbodeName& name() const noexcept { return name_; }

	/// Admit a man as dweller (participation).
	void admit(const Man& man);

	/// Whether this man dwells here (participant).
	bool dwells(const Man& man) const;

	/// Register a Witness as observing this abode.
	void observe(const Witness& witness);

	/// Whether this Witness observes here.
	bool observed_by(const Witness& witness) const;

	/// Witnesses who observe this abode.
	std::vector<std::reference_wrapper<const Witness>> witnesses() const;

private:
	AbodeName name_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_set<const Man*> dwellers_;
	std::unordered_set<const Witness*> witnesses_;
};


} // namespace will::domain
