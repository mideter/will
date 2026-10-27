#pragma once

#include "matter/room.h"
#include "places/gates.h"
#include "places/helm.h"
#include "properties/birth.h"


namespace will::domain {


class Abode;
class Acquaintance;
class Man;


/// A half of an abode: its kind of dwellers enters the rooms set in it. Each has its edge,
/// born with it and never moved: the outer part opens outward at the gates, the inner closes
/// inward at the helm.
class Part {
public:
	virtual ~Part() = default;

	Part(const Part&) = delete;
	Part& operator=(const Part&) = delete;

	const Abode& abode() const noexcept { return abode_; }

	bool admits(const Man& man) const;

	virtual const Place& edge() const = 0;

	virtual matter::Room::Part kept() const = 0;

protected:
	explicit Part(Birth<Abode> birth);

	virtual bool lets_in(const Acquaintance& dweller) const = 0;

private:
	const Abode& abode_;
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


/// The part of the helm, which only friends enter.
class Inner final : public Part {
public:
	Inner(Birth<Abode> birth, id::Place helm);

	const Helm& helm() const noexcept { return helm_; }

	const Place& edge() const override { return helm_; }

	matter::Room::Part kept() const override;

private:
	bool lets_in(const Acquaintance& dweller) const override;

	Helm helm_;
};


} // namespace will::domain
