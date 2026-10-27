#pragma once

#include "matter/behest.h"
#include "properties/birth.h"
#include "places/tie.h"
#include "words/word.h"
#include "values/timestamp.h"

#include <memory>
#include <mutex>


namespace will::domain {


class Deed;
class Life;


/// The testator's word of will in a tie, fulfilled once by the novice's deed, which it knows.
class Behest : public Word {
public:
	Behest(Birth<Tie>, matter::Behest kept);
	Behest(Birth<Life>, matter::Behest kept);

	const Tie& tie() const noexcept { return tie_; }
	Timestamp created_at() const noexcept { return created_at_; }

	const Soul& author() const override;

	std::shared_ptr<const Deed> deed() const;
	void deed(Birth<Tie> birth, std::shared_ptr<const Deed> deed) const;
	void deed(Birth<Life> birth, std::shared_ptr<const Deed> deed) const;

private:
	explicit Behest(matter::Behest kept);

	void fulfil(std::shared_ptr<const Deed> deed) const;

	const Tie& tie_;
	Timestamp created_at_;
	mutable std::unique_ptr<std::mutex> mutex_ = std::make_unique<std::mutex>();
	mutable std::shared_ptr<const Deed> deed_;
};


} // namespace will::domain
