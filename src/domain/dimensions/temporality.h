#pragma once

#include "matter/answer.h"
#include "matter/supplication.h"
#include "matter/dating.h"
#include "matter/embodiment.h"
#include "matter/vessel.h"
#include "matter/effort.h"
#include "matter/execution.h"
#include "identity/word.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/timestamp.h"

#include <cstdint>
#include <vector>


namespace will::domain {


/// Temporality (Временность) — what happens in time: bodies, datings, supplications,
/// executions, efforts.
class Temporality {
public:
	virtual ~Temporality() = default;

	/// One per token.
	virtual matter::Vessel form(DeviceToken token) = 0;

	/// One body per soul, one soul per body.
	virtual matter::Embodiment embody(id::Soul soul, id::Vessel vessel) = 0;

	virtual std::vector<matter::Vessel> vessels() const = 0;

	virtual std::vector<matter::Embodiment> embodiments() const = 0;

	virtual matter::Dating date(id::Word id) = 0;

	/// Skips the undated.
	virtual std::vector<matter::Dating> datings(const std::vector<id::Word>& ids) const = 0;

	/// Refuses (std::logic_error) while an earlier one of the pair awaits its answer.
	virtual matter::Supplication ask(id::Soul suppliant, id::Soul addressee) = 0;

	virtual std::vector<matter::Supplication> supplications(id::Soul addressee) const = 0;

	/// Throws (std::invalid_argument) if none awaits.
	virtual matter::Answer answer(id::Soul suppliant, id::Soul addressee, matter::Answer::Form form) = 0;

	/// Refuses (std::logic_error) if the behest is already fulfilled.
	virtual matter::Execution execute(id::Word deed, id::Word behest) = 0;

	/// Executions in which any of these words is the deed or the fulfilled behest.
	virtual std::vector<matter::Execution> executions(const std::vector<id::Word>& words) const = 0;

	/// Finished at the present instant. Refuses (std::logic_error) if already done.
	virtual matter::Effort exert(id::Word training, std::uint32_t exercise, std::uint32_t approach, Weight weight,
								 std::uint32_t repetitions, Timestamp begun) = 0;

	/// In the order they were finished.
	virtual std::vector<matter::Effort> efforts(const std::vector<id::Word>& trainings) const = 0;
};


} // namespace will::domain
