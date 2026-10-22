#pragma once

#include "matter/room.h"
#include "properties/birth.h"


namespace will::domain {


class Abode;
class Acquaintance;
class Man;


/// A half of an abode: its kind of dwellers enters the cells set in it. Each
/// has its core, which is never moved: the outer the gates, the inner the helm.
class Part {
public:
	virtual ~Part() = default;

	Part(const Part&) = delete;
	Part& operator=(const Part&) = delete;

	const Abode& abode() const noexcept { return abode_; }

	bool admits(const Man& man) const;

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
	explicit Outer(Birth<Abode> birth);

	matter::Room::Part kept() const override;

private:
	bool lets_in(const Acquaintance& dweller) const override;
};


/// The part of the helm, which only friends enter.
class Inner final : public Part {
public:
	explicit Inner(Birth<Abode> birth);

	matter::Room::Part kept() const override;

private:
	bool lets_in(const Acquaintance& dweller) const override;
};


} // namespace will::domain
