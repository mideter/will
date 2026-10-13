#pragma once

#include "identity/soul.h"


namespace will::domain::matter {


/// Answer (Ответ) — to a supplication: accepted, the pair is bound; or rejected.
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
