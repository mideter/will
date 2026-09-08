#pragma once

#include <string_view>


namespace will::domain {


/// Spirit (Дух) — highest, inseparable foundation of the soul.
/// Speech (say) belongs to spirit; only a living Witness can have it heard and fixed.
class Spirit {
public:
	virtual ~Spirit() = default;

	/// Act of will: speak. Outside the living world this fails; Witness overrides.
	virtual void say(std::string_view body) const;

	bool operator==(const Spirit&) const = default;
};


} // namespace will::domain
