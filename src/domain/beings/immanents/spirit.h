#pragma once

#include "acts/contemplation.h"
#include "acts/utterance.h"
#include "beings/heaven.h"
#include "identity/word.h"
#include "identity/soul.h"
#include "ports/eternity.h"
#include "properties/immanent.h"
#include "values/saying.h"

#include <vector>


namespace will::domain {


class Abode;
class Soul;


/// Spirit (Дух) — highest foundation of the soul; immanent to Heaven.
/// Will belongs to spirit (utter); living Witness reveals it as say.
/// Living Man presents the soul to Heaven at birth.
/// Neither copy nor transfer; only Soul may bring forth Spirit.
class Spirit : public Immanent<Heaven> {
public:
	virtual ~Spirit() = default;

	/// Living revelation; implemented by Witness.
	virtual void say(const Saying& saying) const = 0;

protected:
	Spirit() = default;

	/// Living soul known to Heaven. Throws if unknown.
	static const Soul& soul(id::Soul id);

	Utterance utter(const Saying& saying) const;

	std::vector<Utterance> utterances(const std::vector<id::Word>& ids) const;

	bool knows(id::Soul soul_id) const;

	void contemplate(const Abode& abode) const;

	const Contemplation& contemplation() const;

private:
	static Heaven& heaven() { return Heaven::the(); }

	static Eternity& eternity() { return heaven().eternity(); }
};


} // namespace will::domain
