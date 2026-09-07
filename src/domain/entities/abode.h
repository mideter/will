#pragma once

#include "identity/abode.h"
#include "identity/soul.h"
#include "entities/heaven.h"
#include "entities/letter.h"
#include "errors/domain_error.h"
#include "ports/temporality.h"
#include "ports/echo.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>


namespace will::domain {


struct RetoldLetter {
	Letter letter;
	std::string author_name;
	bool is_mine = false;
};


/// Abode (Обитель) — place of communion in the World.
/// Speaks with Temporality; echoes inscribed letters once opened through an Echo.
class Abode {
public:
	static constexpr std::uint32_t MaxRetellLimit = 1000;

	Abode(id::Abode id, Temporality& temporality, Heaven& heaven);

	id::Abode id() const noexcept { return id_; }

	/// Open echo of inscribed letters to participants.
	void echo_through(Echo& echo) noexcept;

	/// Inscribe a letter from a soul into Temporality and echo it.
	Letter inscribe(id::Soul author, std::string_view body);

	/// Retell recent letters for a soul (author names resolved through Heaven).
	std::variant<std::vector<RetoldLetter>, DomainError> retell(id::Soul soul, std::uint32_t limit) const;

private:
	id::Abode id_;
	Temporality& temporality_;
	Heaven& heaven_;
	Echo* echo_ = nullptr;
};


} // namespace will::domain
