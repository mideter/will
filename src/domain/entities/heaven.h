#pragma once

#include "entities/man.h"
#include "entities/soul.h"
#include "ports/eternity.h"
#include "values/device_token.h"

#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


/// Heaven (Небо) — static pole of the one World: index of souls living in men.
/// Speaks with Eternity. World is Heaven and raises / clears this state in its life.
/// Pointers address Soul bases of heap-stable Man (unique_ptr).
/// Lookups hand out those living souls — never snapshots.
class Heaven {
public:
	/// Whether Heaven knows this soul.
	bool knows(id::Soul id) const;

	/// Living soul in the waking cosmos. Throws if unknown.
	const Soul& soul(id::Soul id) const;

protected:
	explicit Heaven(Eternity& eternity);
	~Heaven();

	Heaven(const Heaven&) = delete;
	Heaven& operator=(const Heaven&) = delete;

	/// Give a name and beget a man (with soul and vessel) in Eternity.
	Man beget(const DeviceToken& token);

	/// Recall men from Eternity into the waking cosmos.
	std::vector<Man> remember() const;

	/// Index a soul owned by a heap-stable Man.
	void index(const Soul& soul);

private:
	static Eternity* eternity_;
	static std::mutex mutex_;
	static std::unordered_map<id::Soul, const Soul*> souls_by_id_;
};


} // namespace will::domain
