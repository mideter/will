#pragma once


namespace will::domain {


/// Immanent (Имманентное) — presence in the waking world; neither copy nor transfer.
/// Face of the waking world inherits Immanent<Face> — immanent to itself.
/// Foundations inherit Immanent<Face> — immanent *to* that face; present a Presence
/// level through present<Presence>(). Material may be handed; the living is only
/// brought forth and present()'ed.
template<typename Face>
class Immanent {
public:
	Immanent(const Immanent&) = delete;
	Immanent& operator=(const Immanent&) = delete;
	Immanent(Immanent&&) = delete;
	Immanent& operator=(Immanent&&) = delete;

protected:
	Immanent() = default;
	~Immanent() = default;

	/// Present this object to Face as Presence (inheritance level at the call site).
	template<typename Presence>
	void present() const
	{
		Face::the().present(static_cast<const Presence&>(*this));
	}
};


} // namespace will::domain
