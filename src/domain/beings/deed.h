#pragma once

#include "acts/obedience.h"
#include "acts/shepherding.h"
#include "beings/letter.h"
#include "identity/deed.h"
#include "values/timestamp.h"
#include "values/word.h"

#include <optional>


namespace will::domain {


class Executor;
class Testator;


/// Deed (Дело) — минимальная единица волеизъявления Завещателя Исполнителю;
/// хранится в общем месте Послушания / Ведения (Obedience / Shepherding).
/// Letter fixed in that place; sides from the living pair; open unless restored
/// with timestamps.
class Deed : public Letter {
public:
	Deed(id::Deed id, const Obedience& obedience, Word word, Timestamp created_at,
		 std::optional<Timestamp> executed_at = std::nullopt,
		 std::optional<Timestamp> cancelled_at = std::nullopt);
	Deed(id::Deed id, const Shepherding& shepherding, Word word, Timestamp created_at,
		 std::optional<Timestamp> executed_at = std::nullopt,
		 std::optional<Timestamp> cancelled_at = std::nullopt);

	id::Obedience obedience_id() const noexcept { return obedience_id_; }
	const Testator& testator() const noexcept { return testator_; }
	const Executor& executor() const noexcept { return executor_; }

	const std::optional<Timestamp>& executed_at() const noexcept { return executed_at_; }
	const std::optional<Timestamp>& cancelled_at() const noexcept { return cancelled_at_; }

	bool executed() const noexcept { return executed_at_.has_value(); }
	bool cancelled() const noexcept { return cancelled_at_.has_value(); }
	bool open() const noexcept { return !executed() && !cancelled(); }

private:
	id::Obedience obedience_id_;
	const Testator& testator_;
	const Executor& executor_;
	std::optional<Timestamp> executed_at_;
	std::optional<Timestamp> cancelled_at_;
};


} // namespace will::domain
