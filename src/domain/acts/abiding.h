#pragma once

#include "identity/abode.h"
#include "values/abode_name.h"


namespace will::domain {


/// Abiding — abode id and name recorded in Spatiality; material for living Abode.
/// Not the living Abode (heap presence in Space).
class Abiding {
public:
	Abiding(id::Abode id, AbodeName name);

	id::Abode id() const noexcept { return id_; }
	const AbodeName& name() const noexcept { return name_; }

private:
	id::Abode id_;
	AbodeName name_;
};


} // namespace will::domain
