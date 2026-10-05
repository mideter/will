#pragma once


namespace will::domain {


/// Birth (Рождение) — the living is born only of the living. A constructor that
/// takes Birth<Parent> can be called only by Parent: no one else can make the key,
/// and Parent gains nothing else of what it brings forth. The key carries the
/// parent itself: Parent gives it only with itself.
template<typename Parent>
class Birth {
public:
	/// The one who gives birth.
	const Parent& parent() const noexcept { return parent_; }

private:
	friend Parent;

	explicit Birth(const Parent& parent)
		: parent_(parent)
	{}

	const Parent& parent_;
};


} // namespace will::domain
