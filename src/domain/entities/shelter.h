#pragma once


namespace will::domain {


class Abode;
class Man;


/// Shelter (Убежище) — present cover of a man's soul in an abode (focus).
/// Dweller participation is separate: a man may dwell in many abodes and still
/// have at most one shelter (or none).
class Shelter {
public:
	Shelter(Abode& abode, Man& man) noexcept;

	Abode& abode() noexcept { return abode_; }
	const Abode& abode() const noexcept { return abode_; }

	Man& man() noexcept { return man_; }
	const Man& man() const noexcept { return man_; }

private:
	Abode& abode_;
	Man& man_;
};


} // namespace will::domain
