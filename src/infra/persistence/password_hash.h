#pragma once

#include <string>
#include <string_view>


namespace will {


/** PBKDF2-HMAC-SHA256 password hash for SQLite user storage. */
std::string hash_password(std::string_view password);

bool verify_password_hash(std::string_view stored_hash, std::string_view password);


} // namespace will
