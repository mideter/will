#pragma once

#include "identity/soul.h"
#include "values/soul_name.h"


namespace will::domain::matter {


class Soul {
public:
	Soul(id::Soul id, SoulName name);

	id::Soul id() const noexcept { return id_; }
	const SoulName& name() const noexcept { return name_; }

private:
	id::Soul id_;
	SoulName name_;
};


} // namespace will::domain::matter
