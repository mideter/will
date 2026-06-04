#include "willprotocol.h"

#include <stdexcept>


namespace will {


std::uint32_t TcpFrame::read_u32_be(const unsigned char b[4]) noexcept
{
    return (std::uint32_t(b[0]) << 24u) | (std::uint32_t(b[1]) << 16u) |
           (std::uint32_t(b[2]) << 8u) | std::uint32_t(b[3]);
}


void TcpFrame::append_u32_be(unsigned char b[4], std::size_t payload_len) noexcept
{
    const auto n = static_cast<std::uint32_t>(payload_len);
    b[0] = static_cast<unsigned char>((n >> 24u) & 0xffu);
    b[1] = static_cast<unsigned char>((n >> 16u) & 0xffu);
    b[2] = static_cast<unsigned char>((n >> 8u) & 0xffu);
    b[3] = static_cast<unsigned char>(n & 0xffu);
}


std::vector<char> TcpFrame::encode(const std::vector<char>& payload)
{
    if (payload.size() > MaxPayloadBytes)
        throw std::runtime_error("message exceeds TcpFrame::MaxPayloadBytes");

    unsigned char header_buf[4];
    append_u32_be(header_buf, payload.size());

    std::vector<char> frame;
    frame.reserve(4 + payload.size());
    frame.insert(frame.end(), reinterpret_cast<char*>(header_buf),
                 reinterpret_cast<char*>(header_buf) + 4);
    frame.insert(frame.end(), payload.begin(), payload.end());
    return frame;
}


} // namespace will
