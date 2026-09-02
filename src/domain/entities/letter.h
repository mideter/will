#pragma once

#include "ids/abode_id.h"
#include "ids/letter_id.h"
#include "ids/user_id.h"
#include "values/timestamp.h"

#include <cstddef>
#include <string>


namespace will::domain {


/// Letter (Письмо) — сообщение пользователя в обители. Identity and body are fixed after construction.
class Letter {
public:
    static constexpr std::size_t MaxBodyLength = 4096;

    /// author_id must be non-zero (enforced by UserId). Throws std::invalid_argument if body is empty / too long.
    Letter(LetterId id, AbodeId abode_id, UserId author_id, std::string body, Timestamp created_at);

    LetterId id() const noexcept { return id_; }
    AbodeId abode_id() const noexcept { return abode_id_; }
    UserId author_id() const noexcept { return author_id_; }
    const std::string& body() const noexcept { return body_; }
    Timestamp created_at() const noexcept { return created_at_; }

private:
    LetterId id_;
    AbodeId abode_id_;
    UserId author_id_;
    std::string body_;
    Timestamp created_at_;
};


} // namespace will::domain
