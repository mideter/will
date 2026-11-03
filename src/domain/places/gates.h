#pragma once

#include "places/place.h"
#include "properties/birth.h"
#include "properties/immanent.h"

#include <functional>
#include <vector>


namespace will::domain {


class Contemplation;
class Heaven;
class Man;
class OuterAbode;
class Unborn;


class Gates final : public Place, private Immanent<Heaven> {
public:
	Gates(Birth<OuterAbode> birth, id::Place id);

	const OuterAbode& abode() const noexcept { return abode_; }

	bool leads_to(const Man& who, const Place& next) const override;

	bool within(const OuterAbode& abode) const override;

	bool keeps(const Man& man) const;

	bool open() const;

	void knock(const Contemplation& gaze) const;

	std::vector<std::reference_wrapper<const Man>> knocking() const;

	void admit(const Contemplation& gaze, const Man& man) const;

	const Man& bear(const Contemplation& gaze, const Unborn& unborn) const;

private:
	const OuterAbode& abode_;
};


} // namespace will::domain
