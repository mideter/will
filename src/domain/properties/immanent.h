#pragma once

#include <type_traits>


namespace will::domain {


/// Immanent (Имманентное) — presence in the waking world; neither copy nor transfer.
/// Face (default void): a face of the waking world (Heaven, Earth, Space) inherits
/// Immanent<> — it *is* that presence. Foundations inherit Immanent<Face> — they are
/// immanent *to* that face and present a Presence level through present<Presence>().
/// Material may be handed; the living is only brought forth and present()'ed.
template<typename Face = void>
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
		static_assert(!std::is_void_v<Face>,
					  "Immanent<> faces do not present; foundations use Immanent<Face>");
		Face::the().present(static_cast<const Presence&>(*this));
	}
};


} // namespace will::domain
