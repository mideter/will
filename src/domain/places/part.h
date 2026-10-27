#pragma once

#include "matter/room.h"
#include "places/gates.h"
#include "places/helm.h"
#include "places/room.h"
#include "properties/birth.h"

#include <functional>
#include <memory>
#include <mutex>
#include <vector>


namespace will::domain {


class Abode;
class Acquaintance;
class Man;
class Supplication;
class World;


/// A half of an abode: its rooms lie in it, and its kind of dwellers enters them; one place
/// is reflected by one room of the abode. A kept room is recalled into the part it is kept
/// in: its own by the host, one of a tie by the world once the tie lives. A room moved goes whole from one part into the other. Each has its edge, born
/// with it and never moved: the outer part opens outward at the gates, the inner closes
/// inward at the helm.
class Part {
public:
	virtual ~Part() = default;

	Part(const Part&) = delete;
	Part& operator=(const Part&) = delete;

	const Abode& abode() const noexcept { return abode_; }

	bool admits(const Man& man) const;

	std::vector<std::reference_wrapper<const Room>> rooms() const;

	bool holds(const Room& room) const;

	void recall(Birth<Man> host, const matter::Room& kept) const;
	void recall(Birth<World> world, const matter::Room& kept) const;

	void give(Birth<Abode> birth, const Room& room, const Part& to) const;

	virtual const Place& edge() const = 0;

	virtual matter::Room::Part kept() const = 0;

protected:
	explicit Part(Birth<Abode> birth);

	const Room& lay(const matter::Room& kept) const;

	virtual bool lets_in(const Acquaintance& dweller) const = 0;

private:
	const Abode& abode_;
	mutable std::mutex mutex_;
	mutable std::vector<std::unique_ptr<Room>> rooms_;
};


/// The part of the gates, which neighbours enter.
class Outer final : public Part {
public:
	Outer(Birth<Abode> birth, id::Place gates);

	const Gates& gates() const noexcept { return gates_; }

	const Place& edge() const override { return gates_; }

	matter::Room::Part kept() const override;

private:
	bool lets_in(const Acquaintance& dweller) const override;

	Gates gates_;
};


/// The part of the helm, which only friends enter: a new room is furnished here, its own by
/// its host as the abode is born, and one of a tie as the tie is bound.
class Inner final : public Part {
public:
	Inner(Birth<Abode> birth, id::Place helm);

	const Room& furnish(Birth<Man> host, const matter::Room& kept) const;
	const Room& furnish(Birth<Supplication> supplication, const matter::Room& kept) const;

	const Helm& helm() const noexcept { return helm_; }

	const Place& edge() const override { return helm_; }

	matter::Room::Part kept() const override;

private:
	bool lets_in(const Acquaintance& dweller) const override;

	Helm helm_;
};


} // namespace will::domain
