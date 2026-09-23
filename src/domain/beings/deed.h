#pragma once

#include "acts/tie.h"
#include "identity/deed.h"
#include "values/timestamp.h"
#include "values/word.h"

#include <optional>


namespace will::domain {


/// Deed (Дело) — Word of will in living Tie (Узы);
/// open unless restored with timestamps.
class Deed : public Word {
public:
	Deed(id::Deed id, const Tie& tie, Word word, Timestamp created_at,
		 std::optional<Timestamp> executed_at = std::nullopt,
		 std::optional<Timestamp> cancelled_at = std::nullopt);

	id::Deed id() const noexcept { return id_; }
	const Tie& tie() const noexcept { return tie_; }
	Timestamp created_at() const noexcept { return created_at_; }

	const std::optional<Timestamp>& executed_at() const noexcept { return executed_at_; }
	const std::optional<Timestamp>& cancelled_at() const noexcept { return cancelled_at_; }

	bool executed() const noexcept { return executed_at_.has_value(); }
	bool cancelled() const noexcept { return cancelled_at_.has_value(); }
	bool open() const noexcept { return !executed() && !cancelled(); }

private:
	id::Deed id_;
	const Tie& tie_;
	Timestamp created_at_;
	std::optional<Timestamp> executed_at_;
	std::optional<Timestamp> cancelled_at_;
};


} // namespace will::domain
