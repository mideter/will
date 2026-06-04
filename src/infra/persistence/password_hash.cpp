#include "password_hash.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <array>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>


namespace will {


namespace {


constexpr int Pbkdf2Iterations = 100'000;
constexpr int SaltBytes = 16;
constexpr int DerivedKeyBytes = 32;
constexpr char SchemePrefix[] = "pbkdf2_sha256$";


std::string bytes_to_hex(const unsigned char* data, const std::size_t len)
{
    static constexpr char HexDigits[] = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (std::size_t i = 0; i < len; ++i) {
        out.push_back(HexDigits[(data[i] >> 4) & 0xF]);
        out.push_back(HexDigits[data[i] & 0xF]);
    }
    return out;
}


bool hex_to_bytes(const std::string_view hex, std::vector<unsigned char>& out)
{
    if (hex.size() % 2 != 0)
        return false;

    out.resize(hex.size() / 2);
    auto nibble = [](const char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return -1;
    };

    for (std::size_t i = 0; i < out.size(); ++i) {
        const int hi = nibble(hex[i * 2]);
        const int lo = nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return false;
        out[i] = static_cast<unsigned char>((hi << 4) | lo);
    }
    return true;
}


std::array<unsigned char, DerivedKeyBytes> derive_key(const std::string_view password,
                                                       const unsigned char* salt,
                                                       const int iterations)
{
    std::array<unsigned char, DerivedKeyBytes> key{};
    if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()), salt, SaltBytes,
                          iterations, EVP_sha256(), static_cast<int>(key.size()), key.data()) != 1) {
        throw std::runtime_error("PKCS5_PBKDF2_HMAC failed");
    }
    return key;
}


} // namespace


std::string hash_password(const std::string_view password)
{
    std::array<unsigned char, SaltBytes> salt{};
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1)
        throw std::runtime_error("RAND_bytes failed");

    const auto key = derive_key(password, salt.data(), Pbkdf2Iterations);
    return std::format("{}{}${}${}", SchemePrefix, Pbkdf2Iterations, bytes_to_hex(salt.data(), salt.size()),
                       bytes_to_hex(key.data(), key.size()));
}


bool verify_password_hash(const std::string_view stored_hash, const std::string_view password)
{
    if (!stored_hash.starts_with(SchemePrefix))
        return false;

    const std::string_view payload = stored_hash.substr(std::strlen(SchemePrefix));
    const std::size_t first_dollar = payload.find('$');
    const std::size_t second_dollar = payload.find('$', first_dollar + 1);
    if (first_dollar == std::string_view::npos || second_dollar == std::string_view::npos)
        return false;

    int iterations = 0;
    try {
        iterations = std::stoi(std::string(payload.substr(0, first_dollar)));
    }
    catch (...) {
        return false;
    }
    if (iterations <= 0)
        return false;

    const std::string_view salt_hex = payload.substr(first_dollar + 1, second_dollar - first_dollar - 1);
    const std::string_view key_hex = payload.substr(second_dollar + 1);

    std::vector<unsigned char> salt;
    std::vector<unsigned char> expected_key;
    if (!hex_to_bytes(salt_hex, salt) || salt.size() != SaltBytes)
        return false;
    if (!hex_to_bytes(key_hex, expected_key) || expected_key.size() != DerivedKeyBytes)
        return false;

    const auto actual_key = derive_key(password, salt.data(), iterations);
    return CRYPTO_memcmp(actual_key.data(), expected_key.data(), DerivedKeyBytes) == 0;
}


} // namespace will
