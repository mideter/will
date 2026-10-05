#pragma once

#include "matter/abode.h"
#include "matter/dweller.h"
#include "properties/birth.h"
#include "matter/letter.h"
#include "places/place.h"
#include "values/abode_name.h"

#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Contemplation;
class Letter;
class Man;
class World;


/// Abode (Обитель) — a man's own place, born of its host; in a sense the host
/// is his Abode. Others dwell here as the host admits them, each regarded as
/// an acquaintance, a neighbour or a friend. Place id from Space::point.
/// Rooms come later.
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

	/// How the host regards this dweller; none for the host and for a stranger.
	std::optional<matter::Dweller::Kind> kind(const Man& man) const;

	/// The host and his dwellers dwell here.
	bool dwells(const Man& man) const override;

	/// A letter said here by the one whose gaze rests on it, born living from
	/// the matter the dimensions returned when they kept it; it enters the
	/// recollection of this abode.
	std::shared_ptr<const Letter> inscribe(const Contemplation& gaze, matter::Letter kept) const;

private:
	void admit(const Man& man, const matter::Dweller& kept);

	const Man& host_;
	AbodeName name_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_map<const Man*, matter::Dweller::Kind> dwellers_;

};


} // namespace will::domain
