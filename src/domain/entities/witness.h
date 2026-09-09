#pragma once

#include "entities/abode.h"
#include "entities/letter.h"
#include "entities/man.h"
#include "ports/temporality.h"

#include <cstdint>
#include <string_view>
#include <vector>


namespace will::domain {


/// Witness (Свидетель) — living man; source of will and hearing in the waking world.
/// Created in World::accept from an Eternity Man snapshot; stored as Man.
class Witness : public Man {
public:
	Witness(Man&& man, Abode& abode, Temporality& temporality) noexcept;

	Abode& abode() const noexcept { return *abode_; }

	void say(std::string_view body) const override;

	std::vector<Letter> hear(std::uint32_t limit) const override;

private:
	Abode* abode_;
	Temporality* temporality_;
};


} // namespace will::domain
