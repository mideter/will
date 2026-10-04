#pragma once


namespace will::domain {


/// Birth (Рождение) — the living is born only of the living. A constructor that
/// takes Birth<Parent> can be called only by Parent: no one else can make the key,
/// and Parent gains nothing else of what it brings forth.
template<typename Parent>
class Birth {
	friend Parent;

	Birth() = default;
};


} // namespace will::domain
