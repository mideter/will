#include "willprotocol.h"


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


} // namespace will
