#pragma once


namespace will::domain {


/// Word (Слово) — that which the spirit utters; ontological role.
/// No identity and no Saying here. Living Letter and Deed are Word in the world.
class Word {
public:
	virtual ~Word() = default;

protected:
	Word() = default;
	Word(const Word&) = default;
	Word& operator=(const Word&) = default;
};


} // namespace will::domain
