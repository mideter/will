#pragma once

#include "matter/answer.h"
#include "matter/supplication.h"
#include "matter/dating.h"
#include "matter/embodiment.h"
#include "matter/vessel.h"
#include "matter/execution.h"
#include "identity/word.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/soul_name.h"
#include "values/timestamp.h"

#include <cstdint>
#include <utility>
#include <vector>


namespace will::domain {


/// Temporality (Временность) — what happens in time.
/// Earth speaks with this face (as Heaven speaks with Eternity).
class Temporality {
public:
	virtual ~Temporality() = default;

	/// Keep a new vessel with this token and that this soul dwells in it
	/// (one body per soul, one per token).
	virtual matter::Embodiment embody(id::Soul soul, DeviceToken token) = 0;

	/// All vessels kept in time (for Creation awaken).
	virtual std::vector<matter::Vessel> vessels() const = 0;

	/// All embodiments kept in time (for Creation awaken).
	virtual std::vector<matter::Embodiment> embodiments() const = 0;

	/// Fix a word at the present instant.
	virtual matter::Dating date(id::Word id) = 0;

	/// Datings for the given ids (skips undated).
	virtual std::vector<matter::Dating> datings(const std::vector<id::Word>& ids) const = 0;

	/// Keep a suppliant's supplication to an addressee at the present instant.
	/// Refuses (std::logic_error) while an earlier one of the pair awaits its answer.
	virtual matter::Supplication ask(id::Soul suppliant, id::Soul addressee) = 0;

	/// Supplications to this soul awaiting their answer.
	virtual std::vector<matter::Supplication> supplications(id::Soul addressee) const = 0;

	/// Keep, at the present instant, the answer to the pair's supplication
	/// awaiting it. Throws (std::invalid_argument) if none awaits.
	virtual matter::Answer answer(id::Soul suppliant, id::Soul addressee, matter::Answer::Form form) = 0;

	/// Keep that this deed fulfils that behest.
	/// Refuses (std::logic_error) if the behest is already fulfilled.
	virtual matter::Execution execute(id::Word deed, id::Word behest) = 0;

	/// Executions in which any of these words is the deed or the fulfilled behest.
	virtual std::vector<matter::Execution> executions(const std::vector<id::Word>& words) const = 0;
};


} // namespace will::domain
