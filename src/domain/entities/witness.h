#pragma once

#include "entities/man.h"

#include <string_view>


namespace will::domain {


class Abode;


/// Witness (Свидетель) — living man observing the life of an abode.
/// Created in World::accept from an Eternity Man snapshot; stored as Man.
class Witness : public Man {
public:
	Witness(Man&& man, Abode& abode) noexcept;

	Abode& abode() const noexcept { return *abode_; }

	void say(std::string_view body) const override;

private:
	Abode* abode_;
};


} // namespace will::domain
