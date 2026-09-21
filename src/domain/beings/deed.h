#pragma once

#include "acts/obedience.h"
#include "beings/letter.h"
#include "identity/deed.h"
#include "identity/tie.h"
#include "values/timestamp.h"
#include "values/word.h"

#include <optional>


namespace will::domain {


class Novice;
class Testator;


/// Deed (Дело) — минимальная единица волеизъявления Завещателя Исполнителю;
/// хранится в общем месте Уз (Tie). Letter fixed in that place; sides from
/// the living pair; open unless restored with timestamps.
class Deed : public Letter {
public:
	Deed(id::Deed id, const Obedience& obedience, Word word, Timestamp created_at,
		 std::optional<Timestamp> executed_at = std::nullopt,
		 std::optional<Timestamp> cancelled_at = std::nullopt);
	Deed(id::Deed id, id::Tie place, const Testator& testator, const Novice& novice,
		 Word word, Timestamp created_at,
		 std::optional<Timestamp> executed_at = std::nullopt,
		 std::optional<Timestamp> cancelled_at = std::nullopt);

	id::Tie tie_id() const noexcept { return tie_id_; }
	const Testator& testator() const noexcept { return testator_; }
	const Novice& novice() const noexcept { return novice_; }

	const std::optional<Timestamp>& executed_at() const noexcept { return executed_at_; }
	const std::optional<Timestamp>& cancelled_at() const noexcept { return cancelled_at_; }

	bool executed() const noexcept { return executed_at_.has_value(); }
	bool cancelled() const noexcept { return cancelled_at_.has_value(); }
	bool open() const noexcept { return !executed() && !cancelled(); }

private:
	id::Tie tie_id_;
	const Testator& testator_;
	const Novice& novice_;
	std::optional<Timestamp> executed_at_;
	std::optional<Timestamp> cancelled_at_;
};


} // namespace will::domain
