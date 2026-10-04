#pragma once

#include "identity/word.h"
#include "properties/immanent.h"


namespace will::domain {


class Eternity;


/// Word (Слово) — that which the spirit utters; ontological role, immanent to
/// Eternity: there it is uttered and given its identity, and that part of it
/// never changes. Neither copied nor moved.
/// No Saying here. Living Letter and Deed are Word in the world.
class Word : public Immanent<Eternity> {
public:
	virtual ~Word() = default;

	id::Word id() const noexcept { return id_; }

protected:
	explicit Word(id::Word id) noexcept;

private:
	id::Word id_;
};


} // namespace will::domain
