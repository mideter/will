#pragma once

#include "matter/supplication.h"
#include "relations/contemplation.h"
#include "identity/soul.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Man;
class Place;
class Soul;
class Eternity;
class Gates;
class Spirit;
class Unborn;
class Supplication;


/// The invisible world: the souls, their gazes and the supplications awaiting answer;
/// Eternity is reached from here. A gaze on a room is a gaze on the place it reflects; one
/// asleep has none. Heaven bears a supplication and keeps it until it is answered, oldest
/// first; the world, where Heaven meets the earth, fixes its moment, and gives a soul to a
/// body born at the gates. Whoever holds a gaze or a supplication keeps it from fading while
/// he uses it.
class Heaven : private Immanent<Heaven> {
public:
	bool knows(id::Soul soul_id) const;

	const Soul& soul(id::Soul soul_id) const;

	std::vector<std::reference_wrapper<const Man>> contemplating(const Place& place) const;

	std::vector<std::shared_ptr<const Supplication>> supplications(id::Soul addressee) const;

	std::shared_ptr<const Contemplation> contemplation(const Spirit& eye) const;

protected:
	friend class Spirit;
	friend class Gates;
	friend class Supplication;
	friend class Immanent<Heaven>;

	explicit Heaven(Eternity& eternity);

	~Heaven();

	Eternity& eternity();
	const Eternity& eternity() const;

	std::shared_ptr<const Supplication> keep(matter::Supplication kept);

	virtual void ask(const Spirit& suppliant, const Soul& addressee) = 0;

	virtual const Man& bear(Birth<Gates> gates, const Unborn& unborn) = 0;

private:
	void present(const Soul& soul);

	void contemplate(const Spirit& eye, const Place& place);

	void repose(const Spirit& eye);

	void release(const Supplication& answered);

	static Heaven& the();

	static Heaven* current_;

	Eternity& eternity_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, const Soul*> souls_;
	std::unordered_map<const Spirit*, std::shared_ptr<const Contemplation>> contemplations_;
	std::vector<std::shared_ptr<const Supplication>> supplications_;
};


} // namespace will::domain
