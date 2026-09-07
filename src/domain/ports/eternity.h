#pragma once

#include "entities/man.h"
#include "values/device_token.h"
#include "values/soul_name.h"

#include <vector>


namespace will::domain {


/// Eternity — who endures. Runtime lookups use World (Heaven, Earth, men) while awake.
class Eternity {
public:
	virtual ~Eternity() = default;

	/// Yield men known in eternity (soul and vessel included).
	virtual std::vector<Man> recall() = 0;

	/// Enroll a man in the book of life.
	virtual Man enroll(const DeviceToken& token, SoulName name) = 0;
};


} // namespace will::domain
