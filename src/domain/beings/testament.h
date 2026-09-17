#pragma once

#include "acts/obedience.h"
#include "beings/letter.h"
#include "identity/testament.h"
#include "values/timestamp.h"
#include "values/word.h"

#include <optional>


namespace will::domain {


/// Testament (Завещание) — Letter fixed in an Obedience place; will as bequest.
/// Sides come from the living Obedience; open unless restored with timestamps.
class Testament : public Letter {
public:
	Testament(id::Testament id, const Obedience& obedience, Word word, Timestamp created_at,
			  std::optional<Timestamp> executed_at = std::nullopt,
			  std::optional<Timestamp> cancelled_at = std::nullopt);

	id::Obedience obedience_id() const noexcept { return obedience_id_; }
	const Soul& testator() const noexcept { return testator_; }
	const Soul& executor() const noexcept { return executor_; }

	const std::optional<Timestamp>& executed_at() const noexcept { return executed_at_; }
	const std::optional<Timestamp>& cancelled_at() const noexcept { return cancelled_at_; }

	bool executed() const noexcept { return executed_at_.has_value(); }
	bool cancelled() const noexcept { return cancelled_at_.has_value(); }
	bool open() const noexcept { return !executed() && !cancelled(); }

private:
	id::Obedience obedience_id_;
	const Soul& testator_;
	const Soul& executor_;
	std::optional<Timestamp> executed_at_;
	std::optional<Timestamp> cancelled_at_;
};


} // namespace will::domain
