#include "sha256_otp_hasher.h"

#include <openssl/evp.h>

#include <array>
#include <string>


namespace will {


namespace {


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


} // namespace


std::string Sha256OtpHasher::hash(const std::string_view code, const std::string_view salt) const
{
    const std::string input = std::string(salt) + std::string(code);

    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int digest_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, input.data(), input.size());
    EVP_DigestFinal_ex(ctx, digest.data(), &digest_len);
    EVP_MD_CTX_free(ctx);

    return bytes_to_hex(digest.data(), digest_len);
}


} // namespace will
