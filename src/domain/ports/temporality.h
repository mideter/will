#pragma once

#include "matter/supplication.h"
#include "matter/dating.h"
#include "matter/man.h"
#include "matter/execution.h"
#include "identity/word.h"
#include "identity/soul.h"
#include "identity/tie.h"
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

	/// Keep that this soul dwells in a new vessel; returns matter for World birth.
	virtual matter::Man embody(id::Soul soul, SoulName name, DeviceToken token) = 0;

	/// All men kept in time (for Creation awaken).
	virtual std::vector<matter::Man> men() const = 0;

	/// Fix a word at the present instant.
	virtual matter::Dating date(id::Word id) = 0;

	/// Datings for the given ids (skips undated).
	virtual std::vector<matter::Dating> datings(const std::vector<id::Word>& ids) const = 0;

	/// Keep a suppliant's supplication to an addressee at the present instant.
	/// Refuses (std::logic_error) while an earlier one of the pair is not rejected.
	virtual matter::Supplication ask(id::Soul suppliant, id::Soul addressee) = 0;

	/// Supplications to this soul whose rejection is not kept.
	virtual std::vector<matter::Supplication> supplications(id::Soul addressee) const = 0;

	/// Keep the rejection of the pair's unrejected supplication. Throws if there is none.
	virtual void reject(id::Soul suppliant, id::Soul addressee) = 0;

	/// Keep the execution of a deed at the present instant.
	/// Refuses (std::logic_error) if its matter::Execution is already kept.
	virtual void execute(id::Word deed) = 0;

	/// Executions for the given ids (skips unexecuted).
	virtual std::vector<matter::Execution> executions(const std::vector<id::Word>& ids) const = 0;
};


} // namespace will::domain
