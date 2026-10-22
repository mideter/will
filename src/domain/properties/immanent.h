#pragma once


namespace will::domain {


/// Presence in the waking world: the living is only brought forth and presented, never
/// copied nor handed, as matter may be. It is presented to its horizon as the class that
/// calls, not as the most derived one.
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

	static Horizon& horizon() { return Horizon::the(); }

	template<typename Presence>
	void present() const
	{
		Horizon::the().present(static_cast<const Presence&>(*this));
	}
};


} // namespace will::domain
