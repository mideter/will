#pragma once

#include "entities/man.h"
#include "ports/time.h"
#include "values/device_token.h"
#include "values/soul_name.h"

#include <vector>


namespace will::domain {


/// Eternity (Вечность) — who endures.
/// Time belongs to Eternity; without the eternal there is no present for the temporal.
class Eternity {
public:
	virtual ~Eternity() = default;

	/// Time given by eternity.
	virtual Time& time() = 0;

	/// Men known in eternity (soul and vessel included).
	virtual std::vector<Man> men() = 0;

	/// Enroll a man in the book of life.
	virtual Man enroll(const DeviceToken& token, SoulName name) = 0;
};


} // namespace will::domain
