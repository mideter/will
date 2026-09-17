#pragma once

#include "identity/man.h"


namespace will::domain {


/// Hypostasis (Ипостась) — concrete living presence of a person in the waking cosmos.
/// Named by id::Man; living identity is this address. Neither copy nor transfer.
class Hypostasis {
public:
	id::Man id() const noexcept { return id_; }

	bool operator==(const Hypostasis&) const = default;

protected:
	explicit Hypostasis(id::Man id) noexcept : id_(id) {}
	~Hypostasis() = default;

	Hypostasis(const Hypostasis&) = delete;
	Hypostasis& operator=(const Hypostasis&) = delete;
	Hypostasis(Hypostasis&&) = delete;
	Hypostasis& operator=(Hypostasis&&) = delete;

private:
	id::Man id_;
};


} // namespace will::domain
