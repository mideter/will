#include "wiremessage_codec_internal.h"

#include <array>
#include <cstring>
#include <stdexcept>


namespace will::wiremessage_codec {


void append_u32_be(std::vector<char>& out, const std::uint32_t value)
{
    const auto bytes = TcpFrame::u32_be(value);
    out.insert(out.end(), reinterpret_cast<const char*>(bytes.data()),
               reinterpret_cast<const char*>(bytes.data()) + bytes.size());
}


std::uint32_t read_u32_be_at(const unsigned char* data) noexcept
{
    std::array<unsigned char, 4> buf{};
    std::memcpy(buf.data(), data, buf.size());
    return TcpFrame::read_u32_be(buf);
}


void append_u64_be(std::vector<char>& out, std::uint64_t value)
{
    for (int shift = 56; shift >= 0; shift -= 8)
        out.push_back(static_cast<char>((value >> shift) & 0xffu));
}


std::uint64_t read_u64_be(const unsigned char* data) noexcept
{
    std::uint64_t value = 0;
    for (int i = 0; i < 8; ++i)
        value = (value << 8u) | std::uint64_t(data[i]);
    return value;
}


void append_length_prefixed_string(std::vector<char>& out, std::string_view value)
{
    if (value.size() > MaxAuthFieldBytes)
        throw std::runtime_error("WireMessage: auth field exceeds MaxAuthFieldBytes");

    append_u32_be(out, static_cast<std::uint32_t>(value.size()));
    if (!value.empty())
        out.insert(out.end(), value.begin(), value.end());
}


bool read_length_prefixed_string(std::string_view& field, const std::vector<char>& payload,
                                 std::size_t& offset)
{
    if (offset + 4u > payload.size())
        return false;

    const auto* data = reinterpret_cast<const unsigned char*>(payload.data());
    const std::uint32_t len = read_u32_be_at(data + offset);
    offset += 4u;

    if (len == 0u || len > MaxAuthFieldBytes || offset + len > payload.size())
        return false;

    field = std::string_view(payload.data() + offset, len);
    offset += len;
    return true;
}


std::string format_user_chat_body_for_log(std::string_view body)
{
    std::string out = "UserChat(";
    out += std::to_string(body.size());
    out += " bytes): ";

    static constexpr const char* HexDigits = "0123456789abcdef";
    for (const unsigned char c : body) {
        if (c == '\n') {
            out += "\\n";
            continue;
        }
        if (c == '\r') {
            out += "\\r";
            continue;
        }
        if (c == '\t') {
            out += "\\t";
            continue;
        }
        if (c == '\\') {
            out += "\\\\";
            continue;
        }
        if (c < 32u) {
            out += "\\x";
            out += HexDigits[c >> 4u];
            out += HexDigits[c & 15u];
            continue;
        }
        out += static_cast<char>(c);
    }
    return out;
}


} // namespace will::wiremessage_codec
