#pragma once

#include "beings/spirit.h"
#include "identity/soul.h"
#include "values/soul_name.h"


namespace will::domain {


/// Soul (Душа) — enduring identity of a person; inherits Spirit.
/// Constructed only as the Soul base of a living Man.
class Soul : public Spirit {
public:
	/// Living soul known to Heaven. Throws if unknown.
	static const Soul& of(id::Soul id);

	id::Soul id() const noexcept { return id_; }
	const SoulName& name() const noexcept { return name_; }

	bool operator==(const Soul& other) const noexcept
	{
		return id_ == other.id_ && name_ == other.name_;
	}

protected:
	Soul(id::Soul id, SoulName name);

private:
	id::Soul id_;
	SoulName name_;
};


} // namespace will::domain
