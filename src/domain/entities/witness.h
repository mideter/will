#pragma once

#include "entities/abode.h"
#include "entities/letter.h"
#include "entities/man.h"

#include <cstdint>
#include <string_view>
#include <vector>


namespace will::domain {


/// Witness (Свидетель) — living man observing the life of an abode.
/// Created in World::accept from an Eternity Man snapshot; stored as Man.
class Witness : public Man {
public:
	static constexpr std::uint32_t MaxHearLimit = Abode::MaxLetterLimit;

	Witness(Man&& man, Abode& abode) noexcept;

	Abode& abode() const noexcept { return *abode_; }

	void say(std::string_view body) const override;

	/// Hear recent letters of the observed abode.
	std::vector<Letter> hear(std::uint32_t limit) const;

private:
	Abode* abode_;
};


} // namespace will::domain
