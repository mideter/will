#pragma once


namespace will::domain {


template<typename Parent>
class Birth {
public:
	const Parent& parent() const noexcept { return parent_; }

private:
	friend Parent;

	explicit Birth(const Parent& parent)
		: parent_(parent)
	{}

	const Parent& parent_;
};


} // namespace will::domain
