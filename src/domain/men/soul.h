#pragma once

#include "men/spirit.h"
#include "identity/soul.h"
#include "matter/soul.h"
#include "values/soul_name.h"


namespace will::domain {


/// Soul (Душа) — the enduring identity of a person.
class Soul : public Spirit {
public:
	static const Soul& of(id::Soul id);

	id::Soul id() const noexcept { return id_; }
	const SoulName& name() const noexcept { return name_; }

	bool operator==(const Soul& other) const noexcept;

protected:
	explicit Soul(matter::Soul kept);

private:
	id::Soul id_;
	SoulName name_;
};


} // namespace will::domain
