#pragma once

#include "identity/soul.h"


namespace will::domain::matter {


/// Answer (Ответ) — the addressee's answer to a supplication awaiting it, in one
/// of two forms: accepted (the pair is bound) or rejected; fixed in time.
/// Temporality keeps it; a supplication awaits until its answer is kept.
class Answer {
public:
	enum class Form {
		Accepted,
		Rejected,
	};

	Answer(id::Soul suppliant, id::Soul addressee, Form form);

	id::Soul suppliant() const noexcept { return suppliant_; }
	id::Soul addressee() const noexcept { return addressee_; }
	Form form() const noexcept { return form_; }

private:
	id::Soul suppliant_;
	id::Soul addressee_;
	Form form_;
};


} // namespace will::domain::matter
