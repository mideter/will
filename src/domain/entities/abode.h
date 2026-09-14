#pragma once

#include "identity/abode.h"
#include "entities/man.h"
#include "values/abode_name.h"

#include <memory>
#include <mutex>
#include <unordered_set>


namespace will::domain {


/// Abode (Обитель) — named place of communion in the World.
/// Dwellers: men who participate here (many abodes per man).
/// A living Witness observes one abode; focus implies dwelling there.
/// Movable: Temporality may hand id+name snapshots; dwellers fill in the living World.
class Abode {
public:
	Abode(id::Abode id, AbodeName name);
	Abode(Abode&& other) noexcept;
	Abode& operator=(Abode&& other) noexcept;
	Abode(const Abode&) = delete;
	Abode& operator=(const Abode&) = delete;

	id::Abode id() const noexcept { return id_; }
	const AbodeName& name() const noexcept { return name_; }

	/// Admit a man as dweller (participation).
	void admit(const Man& man);

	/// Whether this man dwells here (participant).
	bool dwells(const Man& man) const;

private:
	id::Abode id_;
	AbodeName name_;
	mutable std::unique_ptr<std::mutex> mutex_;
	std::unordered_set<const Man*> dwellers_;
};


} // namespace will::domain
