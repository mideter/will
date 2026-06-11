#include "tcpframe.h"

#include <stdexcept>


namespace will {


std::array<unsigned char, 4> TcpFrame::u32_be(const std::uint32_t value) noexcept
{
    return {
        static_cast<unsigned char>((value >> 24u) & 0xffu),
        static_cast<unsigned char>((value >> 16u) & 0xffu),
        static_cast<unsigned char>((value >> 8u) & 0xffu),
        static_cast<unsigned char>(value & 0xffu),
    };
}


std::uint32_t TcpFrame::read_u32_be(const std::array<unsigned char, 4>& bytes) noexcept
{
    return (std::uint32_t(bytes[0]) << 24u) | (std::uint32_t(bytes[1]) << 16u) |
           (std::uint32_t(bytes[2]) << 8u) | std::uint32_t(bytes[3]);
}


std::vector<char> TcpFrame::encode(const std::vector<char>& payload)
{
    if (payload.size() > MaxPayloadBytes)
        throw std::runtime_error("message exceeds TcpFrame::MaxPayloadBytes");

    const auto header = u32_be(static_cast<std::uint32_t>(payload.size()));

    std::vector<char> frame;
    frame.reserve(4 + payload.size());
    frame.insert(frame.end(), reinterpret_cast<const char*>(header.data()),
                 reinterpret_cast<const char*>(header.data()) + header.size());
    frame.insert(frame.end(), payload.begin(), payload.end());

    return frame;
}


} // namespace will
