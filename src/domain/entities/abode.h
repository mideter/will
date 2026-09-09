#pragma once

#include "identity/abode.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "entities/man.h"
#include "ports/temporality.h"
#include "values/abode_name.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>


namespace will::domain {


struct RetoldLetter {
	Letter letter;
	std::string author_name;
	bool is_mine = false;
};


/// Abode (Обитель) — named place of communion in the World.
/// Dwellers: men who participate here (many abodes per man).
/// A living Witness observes one abode; focus implies dwelling there.
/// Speaks with Temporality.
class Abode {
public:
	static constexpr std::uint32_t MaxLetterLimit = 1000;

	Abode(id::Abode id, AbodeName name, Temporality& temporality, Heaven& heaven);

	id::Abode id() const noexcept { return id_; }
	const AbodeName& name() const noexcept { return name_; }

	const Heaven& heaven() const noexcept { return heaven_; }

	/// Admit a man as dweller (participation).
	void admit(const Man& man);

	/// Whether this man dwells here (participant).
	bool dwells(const Man& man) const;

	/// Inscribe what a man observing this abode has said (living Witness).
	void inscribe(const Man& author, std::string_view body);

	/// Recent letters fixed in this abode (capped). For Witness::hear.
	std::vector<Letter> letters(std::uint32_t limit) const;

private:
	id::Abode id_;
	AbodeName name_;
	Temporality& temporality_;
	Heaven& heaven_;
	mutable std::mutex mutex_;
	std::unordered_set<const Man*> dwellers_;
};


} // namespace will::domain
