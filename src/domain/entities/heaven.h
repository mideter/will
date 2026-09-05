#pragma once

#include "entities/man.h"
#include "entities/soul.h"
#include "ports/eternity.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>


namespace will::domain {


/// Heaven (Небо) — runtime index of souls living in men. Speaks with Eternity.
/// Living access is through World (Мир), which is Heaven.
/// Pointers address Soul/Vessel bases of heap-stable Man (unique_ptr).
class Heaven {
public:
	std::optional<Soul> find_by_id(id::Soul id) const;

protected:
	explicit Heaven(Eternity& eternity);

	/// Give a name and birth man (with soul and vessel) in Eternity.
	Man birth_man(const DeviceToken& token);

	/// Recall men from Eternity into the waking cosmos.
	std::vector<Man> remember() const;

	/// Index a soul owned by a heap-stable Man.
	void index(const Soul& soul);

private:
	Eternity& eternity_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Soul, const Soul*> souls_by_id_;
};


} // namespace will::domain
