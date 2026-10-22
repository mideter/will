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


/// Heaven (Небо) — the invisible world: the souls, their gazes and the supplications
/// awaiting answer; Eternity is reached from here.
class Heaven : private Immanent<Heaven> {
public:
	bool knows(id::Soul soul_id) const;

	const Soul& soul(id::Soul soul_id) const;

	/// On the place itself, or on a room reflecting it.
	std::vector<std::reference_wrapper<const Man>> contemplating(const Place& place) const;

	/// Oldest first. Whoever holds one keeps it from fading while he uses it.
	std::vector<std::shared_ptr<const Supplication>> supplications(id::Soul addressee) const;

	/// Null while asleep. Whoever holds it keeps it from fading while he uses it.
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

	/// Heaven bears a supplication and keeps it until it is answered.
	std::shared_ptr<const Supplication> keep(matter::Supplication kept);

	/// A spirit asks one to become its testator: the world, where Heaven meets the earth,
	/// fixes its moment and Heaven keeps it.
	virtual void ask(const Spirit& suppliant, const Soul& addressee) = 0;

	/// A waiting body is born at the gates, of their host: the world gives it a soul.
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
