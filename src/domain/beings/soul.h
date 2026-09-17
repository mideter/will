#pragma once

#include "beings/spirit.h"
#include "identity/soul.h"
#include "values/soul_name.h"


namespace will::domain {


/// Soul (Душа) — enduring identity of a person; inherits Spirit.
class Soul : public Spirit {
public:
	Soul(id::Soul id, SoulName name);

	id::Soul id() const noexcept { return id_; }
	const SoulName& name() const noexcept { return name_; }

	bool operator==(const Soul& other) const noexcept
	{
		return id_ == other.id_ && name_ == other.name_;
	}

private:
	id::Soul id_;
	SoulName name_;
};


} // namespace will::domain
