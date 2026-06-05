#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>


namespace will {

    
/** Framing helpers: uint32 big-endian length prefix + payload. */
class TcpFrame {
public:
    /** Payload octets per frame after the 4-byte length prefix. */
    static constexpr std::size_t MaxPayloadBytes = 1u << 20;

    static std::array<unsigned char, 4> u32_be(std::uint32_t value) noexcept;
    static std::uint32_t read_u32_be(const std::array<unsigned char, 4>& bytes) noexcept;

    /** 4-byte BE length prefix + {@code payload}. Throws if payload is too large. */
    static std::vector<char> encode(const std::vector<char>& payload);

    TcpFrame() = delete;
};


} // namespace will
