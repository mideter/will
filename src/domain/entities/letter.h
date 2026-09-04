#pragma once

#include "identity/abode.h"
#include "identity/soul.h"
#include "identity/letter.h"
#include "values/timestamp.h"

#include <cstddef>
#include <string>


namespace will::domain {


/// Letter (Письмо) — сообщение пользователя в обители. Identity and body are fixed after construction.
class Letter {
public:
	static constexpr std::size_t MaxBodyLength = 4096;

	/// author_id must be non-zero (enforced by id::Soul). Throws std::invalid_argument if body is empty / too long.
	Letter(id::Letter id, id::Abode abode_id, id::Soul author_id, std::string body, Timestamp created_at);

	id::Letter id() const noexcept { return id_; }
	id::Abode abode_id() const noexcept { return abode_id_; }
	id::Soul author_id() const noexcept { return author_id_; }
	const std::string& body() const noexcept { return body_; }
	Timestamp created_at() const noexcept { return created_at_; }

private:
	id::Letter id_;
	id::Abode abode_id_;
	id::Soul author_id_;
	std::string body_;
	Timestamp created_at_;
};


} // namespace will::domain
