#pragma once

#include "entities/letter.h"

#include <cstdint>
#include <string_view>
#include <vector>


namespace will::domain {


/// Spirit (Дух) — highest, inseparable foundation of the soul.
/// Will (say) and hearing (hear) belong to spirit; only a living Witness enacts them.
class Spirit {
public:
	static constexpr std::uint32_t MaxHearLimit = 1000;

	virtual ~Spirit() = default;

	/// Act of will: speak. Outside the living world this fails; Witness overrides.
	virtual void say(std::string_view body) const;

	/// Act of hearing: recent letters of the observed abode. Outside the living world this fails.
	virtual std::vector<Letter> hear(std::uint32_t limit) const;

	bool operator==(const Spirit&) const = default;
};


} // namespace will::domain
