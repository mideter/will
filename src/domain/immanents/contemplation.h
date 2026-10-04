#pragma once

#include "immanents/abode.h"
#include "properties/immanent.h"

#include <memory>
#include <mutex>
#include <vector>


namespace will::domain {


class Heaven;
class Letter;
class Witness;


/// Contemplation (Созерцание) — the gaze of a soul upon an Abode; immanent to Heaven.
/// Heaven keeps it, one per soul, from the moment the soul's man wakes or turns
/// to an abode until he turns elsewhere or falls asleep.
/// The letters of an abode live while they are beheld: each gaze holds them,
/// all gazes upon the abode share the same ones, the abode only remembers them.
class Contemplation : public Immanent<Heaven> {
public:
	const Witness& who() const noexcept { return who_; }
	const Abode& abode() const noexcept { return abode_; }

	/// Living letters of the contemplated abode, oldest first.
	std::vector<std::shared_ptr<const Letter>> letters() const;

	/// Behold a letter newly said in the contemplated abode.
	void behold(std::shared_ptr<const Letter> letter) const;

private:
	friend class Heaven;

	Contemplation(const Witness& who, const Abode& abode);

	const Witness& who_;
	const Abode& abode_;

	mutable std::mutex mutex_;
	mutable std::vector<std::shared_ptr<const Letter>> letters_;
};


} // namespace will::domain
