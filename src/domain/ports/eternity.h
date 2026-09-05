#pragma once

#include "entities/man.h"
#include "values/device_token.h"
#include "values/soul_name.h"

#include <vector>


namespace will::domain {


/// Eternal memory — durable storage. Runtime lookups use World (Heaven, Earth, men) in memory.
class Eternity {
public:
	virtual ~Eternity() = default;

	/// Yield men known in eternal memory (soul and vessel included).
	virtual std::vector<Man> recall() = 0;
	virtual Man insert_man(const DeviceToken& token, SoulName name) = 0;
};


} // namespace will::domain
