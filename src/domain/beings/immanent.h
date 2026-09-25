#pragma once


namespace will::domain {


/// Immanent (Имманентное) — abides as living presence; neither copy nor transfer.
/// Material may be handed; the living is only brought forth and present()'ed.
class Immanent {
public:
	Immanent(const Immanent&) = delete;
	Immanent& operator=(const Immanent&) = delete;
	Immanent(Immanent&&) = delete;
	Immanent& operator=(Immanent&&) = delete;

protected:
	Immanent() = default;
	~Immanent() = default;
};


} // namespace will::domain
