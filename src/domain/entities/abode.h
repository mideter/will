#pragma once

#include "identity/abode.h"
#include "identity/soul.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "entities/man.h"
#include "entities/shelter.h"
#include "errors/domain_error.h"
#include "ports/temporality.h"
#include "values/abode_name.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>


namespace will::domain {


struct RetoldLetter {
	Letter letter;
	std::string author_name;
	bool is_mine = false;
};


/// Abode (Обитель) — named place of communion in the World.
/// Dwellers: men who participate here (many abodes per man).
/// Shelters: present focus covers (factory + store); at most one per man in the world.
/// Speaks with Temporality.
class Abode {
public:
	static constexpr std::uint32_t MaxRetellLimit = 1000;

	Abode(id::Abode id, AbodeName name, Temporality& temporality, Heaven& heaven);

	id::Abode id() const noexcept { return id_; }
	const AbodeName& name() const noexcept { return name_; }

	/// Admit a man as dweller (participation). Not the same as under shelter.
	void admit(const Man& man);

	/// Whether this man dwells here (participant).
	bool dwells(const Man& man) const;

	/// Grant shelter to a dwelling man (moves focus if sheltered elsewhere).
	Shelter& shelter(Man& man);

	/// Release this abode's shelter for the man, if any.
	void release_shelter(Man& man);

	/// Whether this man's present shelter is in this abode.
	bool shelters(const Man& man) const;

	/// Inscribe a letter from a man sheltered in this abode.
	Letter inscribe(const Man& author, std::string_view body);

	/// Retell recent letters for a soul (author names resolved through Heaven).
	std::variant<std::vector<RetoldLetter>, DomainError> retell(id::Soul soul, std::uint32_t limit) const;

private:
	id::Abode id_;
	AbodeName name_;
	Temporality& temporality_;
	Heaven& heaven_;
	mutable std::mutex mutex_;
	std::unordered_set<const Man*> dwellers_;
	std::unordered_map<const Man*, std::unique_ptr<Shelter>> shelters_;
};


} // namespace will::domain
