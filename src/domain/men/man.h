#pragma once

#include "matter/fatherhood.h"
#include "matter/man.h"
#include "places/abode.h"
#include "men/novice.h"
#include "men/testator.h"
#include "men/witness.h"

#include <atomic>
#include <memory>


namespace will::domain {


class Gates;
class Helm;
class World;


/// Man (Человек) — the living man: witness, novice and testator at once.
class Man final : public Witness, public Novice, public Testator {
public:
	Man(Birth<World>, matter::Man kept);
	~Man() override;

	/// The one soul by this id, if it is a living man's.
	static const Man& of(id::Soul id);

	Abode& abode() const noexcept override { return *abode_; }

	/// He becomes an acquaintance of the host.
	void admit(Birth<Gates> gates, const Man& man) const;

	void regard(Birth<Helm> helm, const Man& dweller, matter::Dweller::Kind kind) const;

	void arrange(Birth<Helm> helm, const Cell& cell, const Part& part) const;

	const Man* father(matter::Fatherhood::Line line) const noexcept;

	void descend(Birth<World> birth, const Man& father, const matter::Fatherhood& kept) const;

private:
	std::unique_ptr<Abode> abode_;
	mutable std::atomic<const Man*> father_by_flesh_ = nullptr;
	mutable std::atomic<const Man*> father_by_spirit_ = nullptr;
};


/// The man whose role this is: a role lives only in a man.
inline const Man& man_of(const Witness& role) noexcept { return static_cast<const Man&>(role); }
inline const Man& man_of(const Novice& role) noexcept { return static_cast<const Man&>(role); }
inline const Man& man_of(const Testator& role) noexcept { return static_cast<const Man&>(role); }


} // namespace will::domain
