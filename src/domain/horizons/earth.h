#pragma once

#include "properties/immanent.h"
#include "identity/vessel.h"
#include "values/device_token.h"

#include <mutex>
#include <optional>
#include <unordered_map>


namespace will::domain {


class Vessel;
class Temporality;
class Spatiality;


/// The visible world: the bodies, and the dimensions that keep matter.
class Earth : private Immanent<Earth> {
public:
	bool knows(id::Vessel id) const;

	const Vessel& vessel(id::Vessel id) const;

protected:
	friend class Dust;
	friend class Immanent<Earth>;

	Earth(Temporality& temporality, Spatiality& spatiality);

	~Earth();

	std::optional<id::Vessel> id_of(const DeviceToken& token) const;

	Temporality& temporality() noexcept { return temporality_; }
	const Temporality& temporality() const noexcept { return temporality_; }

	Spatiality& spatiality() noexcept { return spatiality_; }
	const Spatiality& spatiality() const noexcept { return spatiality_; }

private:
	void present(const Vessel& vessel);

	static Earth& the();

	static Earth* current_;

	Temporality& temporality_;
	Spatiality& spatiality_;
	mutable std::mutex mutex_;
	std::unordered_map<id::Vessel, const Vessel*> vessels_;
	std::unordered_map<DeviceToken, id::Vessel> id_by_token_;
};


} // namespace will::domain
