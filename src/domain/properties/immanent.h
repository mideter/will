#pragma once


namespace will::domain {


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
