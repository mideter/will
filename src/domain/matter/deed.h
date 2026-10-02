#pragma once

#include "identity/word.h"
#include "matter/dating.h"
#include "matter/execution.h"
#include "matter/placement.h"
#include "matter/utterance.h"

#include <optional>


namespace will::domain::matter {


/// Matter of living Deed (Дело) — the parts of a Word of will gathered from the
/// three dimensions, and its Execution once that is kept in Temporality.
/// Composite: no single dimension keeps it. The parts share the word id.
class Deed {
public:
	Deed(Utterance utterance, Placement placement, Dating dating,
		 std::optional<Execution> execution = std::nullopt);

	id::Word id() const noexcept { return utterance_.id(); }
	const Utterance& utterance() const noexcept { return utterance_; }
	const Placement& placement() const noexcept { return placement_; }
	const Dating& dating() const noexcept { return dating_; }
	const std::optional<Execution>& execution() const noexcept { return execution_; }

private:
	Utterance utterance_;
	Placement placement_;
	Dating dating_;
	std::optional<Execution> execution_;
};


} // namespace will::domain::matter
