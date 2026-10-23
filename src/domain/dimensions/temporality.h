#pragma once

#include "matter/answer.h"
#include "matter/supplication.h"
#include "matter/dating.h"
#include "matter/embodiment.h"
#include "matter/vessel.h"
#include "matter/effort.h"
#include "matter/underway.h"
#include "matter/execution.h"
#include "matter/review.h"
#include "identity/word.h"
#include "identity/soul.h"
#include "identity/vessel.h"
#include "values/device_token.h"
#include "values/timestamp.h"

#include <cstdint>
#include <vector>


namespace will::domain {


/// What happens in time: bodies, datings, supplications, executions, reviews, efforts. One body
/// per token, one soul per body. Of a pair, one supplication awaits at a time, and only
/// an awaiting one is answered. A behest is fulfilled once; its deed is reviewed once, at the
/// present instant. A training has at most one
/// approach underway, begun and ended at the present instant; it ends once, and its effort is
/// finished when it ended, or when it is told if it did not. Efforts come in the order they
/// were finished. What it does not know it passes over; what would break these
/// it refuses.
class Temporality {
public:
	virtual ~Temporality() = default;

	virtual matter::Vessel form(DeviceToken token) = 0;

	virtual matter::Embodiment embody(id::Soul soul, id::Vessel vessel) = 0;

	virtual std::vector<matter::Vessel> vessels() const = 0;

	virtual std::vector<matter::Embodiment> embodiments() const = 0;

	virtual matter::Dating date(id::Word id) = 0;

	virtual std::vector<matter::Dating> datings(const std::vector<id::Word>& ids) const = 0;

	virtual matter::Supplication ask(id::Soul suppliant, id::Soul addressee) = 0;

	virtual std::vector<matter::Supplication> supplications(id::Soul addressee) const = 0;

	virtual matter::Answer answer(id::Soul suppliant, id::Soul addressee, matter::Answer::Form form) = 0;

	virtual matter::Execution execute(id::Word deed, id::Word behest) = 0;

	virtual std::vector<matter::Execution> executions(const std::vector<id::Word>& words) const = 0;

	virtual matter::Review review(id::Word deed) = 0;

	virtual std::vector<matter::Review> reviews(const std::vector<id::Word>& deeds) const = 0;

	virtual matter::Underway begin(id::Word training, std::uint32_t exercise, std::uint32_t approach) = 0;

	virtual std::vector<matter::Underway> underways(const std::vector<id::Word>& trainings) const = 0;

	virtual matter::Underway end(const matter::Underway& doing) = 0;

	virtual matter::Effort exert(const matter::Underway& doing, Weight weight, std::uint32_t repetitions,
								 Strain strain) = 0;

	virtual std::vector<matter::Effort> efforts(const std::vector<id::Word>& trainings) const = 0;
};


} // namespace will::domain
