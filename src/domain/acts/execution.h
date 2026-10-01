#pragma once

#include "identity/word.h"
#include "values/timestamp.h"


namespace will::domain {


/// Execution (Исполнение) — Deed carried out, fixed in time; matter for living Deed.
/// Temporality keeps Executions; a Deed is open until its Execution is kept.
class Execution {
public:
	Execution(id::Word id, Timestamp executed_at);

	id::Word id() const noexcept { return id_; }
	Timestamp executed_at() const noexcept { return executed_at_; }

private:
	id::Word id_;
	Timestamp executed_at_;
};


} // namespace will::domain
