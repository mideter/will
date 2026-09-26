#pragma once


namespace will::domain {


/// Immanent (Имманентное) — presence in the waking world; neither copy nor transfer.
/// Heaven, Earth, and Space inherit Immanent of themselves.
/// Spirit is immanent to Heaven, Dust to Earth, Place to Space; they present a
/// Presence level through present<Presence>(). Material may be handed; the living
/// is only brought forth and present()'ed.
template<typename Horizon>
class Immanent {
public:
	Immanent(const Immanent&) = delete;
	Immanent& operator=(const Immanent&) = delete;
	Immanent(Immanent&&) = delete;
	Immanent& operator=(Immanent&&) = delete;

protected:
	Immanent() = default;
	~Immanent() = default;

	/// Present this object as Presence (inheritance level at the call site).
	template<typename Presence>
	void present() const
	{
		Horizon::the().present(static_cast<const Presence&>(*this));
	}
};


} // namespace will::domain
