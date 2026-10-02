#pragma once

#include "acts/asking.h"
#include "acts/dating.h"
#include "acts/embodiment.h"
#include "acts/execution.h"
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
	virtual Embodiment embody(id::Soul soul, SoulName name, DeviceToken token) = 0;

	/// All embodiments kept in time (for Creation awaken).
	virtual std::vector<Embodiment> embodiments() const = 0;

	/// Fix a word at the present instant.
	virtual Dating date(id::Word id) = 0;

	/// Datings for the given ids (skips undated).
	virtual std::vector<Dating> datings(const std::vector<id::Word>& ids) const = 0;

	/// Keep a suppliant's asking of an addressee at the present instant.
	/// Refuses (std::logic_error) while an earlier asking of the pair is not rejected.
	virtual Asking ask(id::Soul suppliant, id::Soul addressee) = 0;

	/// Askings of this soul whose rejection is not kept.
	virtual std::vector<Asking> askings(id::Soul addressee) const = 0;

	/// Keep the rejection of the pair's unrejected asking. Throws if there is none.
	virtual void reject(id::Soul suppliant, id::Soul addressee) = 0;

	/// Keep the execution of a deed at the present instant.
	/// Refuses (std::logic_error) if its Execution is already kept.
	virtual void execute(id::Word deed) = 0;

	/// Executions for the given ids (skips unexecuted).
	virtual std::vector<Execution> executions(const std::vector<id::Word>& ids) const = 0;
};


} // namespace will::domain
