#pragma once

#include "matter/abode.h"
#include "matter/dweller.h"
#include "properties/birth.h"
#include "matter/letter.h"
#include "places/place.h"
#include "values/abode_name.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Acquaintance;
class Contemplation;
class Letter;
class Man;
class World;


/// Abode (Обитель) — a man's own place, born of its host; in a sense the host
/// is his Abode. Others dwell here as the host admits them: each is born an
/// Acquaintance, a Neighbour or a Friend, and sees as his kind does; the host
/// sees all. Place id from Space::point. Rooms come later.
class Abode : public Place {
public:
	Abode(Birth<Man> birth, matter::Abode kept);

	const AbodeName& name() const noexcept { return name_; }

	const Man& host() const noexcept { return host_; }

	/// A man dwells here as his dweller matter says: the host admits him or
	/// regards him anew, or the World recalls him on awakening. Throws if the
	/// matter is of another abode or another soul, or the man is the host.
	void admit(Birth<Man> birth, const Man& man, const matter::Dweller& kept);
	void admit(Birth<World> birth, const Man& man, const matter::Dweller& kept);

	/// This man as a dweller here; none for the host and for a stranger.
	std::shared_ptr<const Acquaintance> dweller(const Man& man) const;

	/// The host and his dwellers dwell here.
	bool dwells(const Man& man) const override;

	/// To its host the abode shows all; to a dweller, what his kind sees.
	bool shows(const Man& who, const Word& word) const override;

	/// A letter said here by the one whose gaze rests on it, born living from
	/// the matter the dimensions returned when they kept it; it enters the
	/// recollection of this abode.
	std::shared_ptr<const Letter> inscribe(const Contemplation& gaze, matter::Letter kept) const;

private:
	void admit(const Man& man, const matter::Dweller& kept);

	const Man& host_;
	AbodeName name_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_map<const Man*, std::shared_ptr<const Acquaintance>> dwellers_;

};


} // namespace will::domain
