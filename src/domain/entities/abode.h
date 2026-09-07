#pragma once

#include "identity/abode.h"
#include "identity/soul.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "entities/soul.h"
#include "errors/domain_error.h"
#include "ports/temporality.h"
#include "values/abode_name.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
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
/// Composition is a subset of enduring souls (stable references into World's men).
/// Speaks with Temporality.
class Abode {
public:
	static constexpr std::uint32_t MaxRetellLimit = 1000;

	Abode(id::Abode id, AbodeName name, Temporality& temporality, Heaven& heaven);

	id::Abode id() const noexcept { return id_; }
	const AbodeName& name() const noexcept { return name_; }

	/// Admit an enduring soul into this abode's living composition.
	void admit(const Soul& soul);

	/// Whether this abode shelters the soul among its members.
	bool shelters(const Soul& soul) const;

	/// Inscribe a letter from a soul into Temporality.
	Letter inscribe(id::Soul author, std::string_view body);

	/// Retell recent letters for a soul (author names resolved through Heaven).
	std::variant<std::vector<RetoldLetter>, DomainError> retell(id::Soul soul, std::uint32_t limit) const;

private:
	id::Abode id_;
	AbodeName name_;
	Temporality& temporality_;
	Heaven& heaven_;
	mutable std::mutex mutex_;
	std::unordered_set<const Soul*> members_;
};


} // namespace will::domain
