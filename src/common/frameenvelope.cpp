#include "frameenvelope.h"

#include <cstring>
#include <stdexcept>


namespace will {


void FrameEnvelope::append_u64_be(unsigned char b[8], std::uint64_t v) noexcept
{
    b[0] = static_cast<unsigned char>((v >> 56u) & 0xffu);
    b[1] = static_cast<unsigned char>((v >> 48u) & 0xffu);
    b[2] = static_cast<unsigned char>((v >> 40u) & 0xffu);
    b[3] = static_cast<unsigned char>((v >> 32u) & 0xffu);
    b[4] = static_cast<unsigned char>((v >> 24u) & 0xffu);
    b[5] = static_cast<unsigned char>((v >> 16u) & 0xffu);
    b[6] = static_cast<unsigned char>((v >> 8u) & 0xffu);
    b[7] = static_cast<unsigned char>(v & 0xffu);
}


std::uint64_t FrameEnvelope::read_u64_be(const unsigned char b[8]) noexcept
{
    return (std::uint64_t(b[0]) << 56u) | (std::uint64_t(b[1]) << 48u) |
           (std::uint64_t(b[2]) << 40u) | (std::uint64_t(b[3]) << 32u) |
           (std::uint64_t(b[4]) << 24u) | (std::uint64_t(b[5]) << 16u) |
           (std::uint64_t(b[6]) << 8u) | std::uint64_t(b[7]);
}


void FrameEnvelope::append_u32_be(unsigned char b[4], std::uint32_t v) noexcept
{
    b[0] = static_cast<unsigned char>((v >> 24u) & 0xffu);
    b[1] = static_cast<unsigned char>((v >> 16u) & 0xffu);
    b[2] = static_cast<unsigned char>((v >> 8u) & 0xffu);
    b[3] = static_cast<unsigned char>(v & 0xffu);
}


std::uint32_t FrameEnvelope::read_u32_be(const unsigned char b[4]) noexcept
{
    return (std::uint32_t(b[0]) << 24u) | (std::uint32_t(b[1]) << 16u) |
           (std::uint32_t(b[2]) << 8u) | std::uint32_t(b[3]);
}


std::vector<char> FrameEnvelope::encode_chat(std::uint64_t client_msg_id, std::string_view utf8_body)
{
    const std::size_t total = ChatMinPayloadBytes + utf8_body.size();
    if (total > TcpFrame::MaxPayloadBytes)
        throw std::runtime_error("FrameEnvelope::encode_chat: payload exceeds TcpFrame::MaxPayloadBytes");

    std::vector<char> out(total);
    out[0] = static_cast<char>(KindChat);
    unsigned char id_be[8];
    append_u64_be(id_be, client_msg_id);
    for (int i = 0; i < 8; ++i)
        out[static_cast<std::size_t>(1 + i)] = static_cast<char>(id_be[i]);
    if (!utf8_body.empty())
        std::memcpy(out.data() + ChatMinPayloadBytes, utf8_body.data(), utf8_body.size());
    return out;
}


std::vector<char> FrameEnvelope::encode_server_ack(std::uint64_t client_msg_id,
                                                    std::uint32_t target_peer_count,
                                                    std::uint32_t success_send_count)
{
    std::vector<char> out(ServerAckPayloadBytes);
    out[0] = static_cast<char>(KindServerAck);
    unsigned char id_be[8];
    append_u64_be(id_be, client_msg_id);
    for (int i = 0; i < 8; ++i)
        out[static_cast<std::size_t>(1 + i)] = static_cast<char>(id_be[i]);

    unsigned char t_be[4];
    unsigned char s_be[4];
    append_u32_be(t_be, target_peer_count);
    append_u32_be(s_be, success_send_count);
    for (int i = 0; i < 4; ++i)
        out[9 + static_cast<std::size_t>(i)] = static_cast<char>(t_be[i]);
    for (int i = 0; i < 4; ++i)
        out[13 + static_cast<std::size_t>(i)] = static_cast<char>(s_be[i]);
    return out;
}


bool FrameEnvelope::try_decode_chat(const std::vector<char>& payload,
                                    std::uint64_t& client_msg_id_out,
                                    std::string_view& utf8_body_out) noexcept
{
    if (payload.size() < ChatMinPayloadBytes)
        return false;
    const auto* p = reinterpret_cast<const unsigned char*>(payload.data());
    if (p[0] != KindChat)
        return false;
    unsigned char id_bytes[8];
    for (int i = 0; i < 8; ++i)
        id_bytes[i] = p[1 + i];
    client_msg_id_out = read_u64_be(id_bytes);
    utf8_body_out = std::string_view(payload.data() + ChatMinPayloadBytes, payload.size() - ChatMinPayloadBytes);
    return true;
}


bool FrameEnvelope::try_decode_server_ack(const std::vector<char>& payload,
                                          std::uint64_t& client_msg_id_out,
                                          std::uint32_t& target_peer_count_out,
                                          std::uint32_t& success_send_count_out) noexcept
{
    if (payload.size() != ServerAckPayloadBytes)
        return false;
    const auto* p = reinterpret_cast<const unsigned char*>(payload.data());
    if (p[0] != KindServerAck)
        return false;
    unsigned char id_bytes[8];
    for (int i = 0; i < 8; ++i)
        id_bytes[i] = p[1 + i];
    client_msg_id_out = read_u64_be(id_bytes);
    unsigned char t_be[4];
    unsigned char s_be[4];
    for (int i = 0; i < 4; ++i)
        t_be[i] = p[9 + i];
    for (int i = 0; i < 4; ++i)
        s_be[i] = p[13 + i];
    target_peer_count_out = read_u32_be(t_be);
    success_send_count_out = read_u32_be(s_be);
    return true;
}


bool FrameEnvelope::looks_like_v2_chat(const std::vector<char>& payload) noexcept
{
    return payload.size() >= ChatMinPayloadBytes &&
           static_cast<unsigned char>(payload[0]) == KindChat;
}


bool FrameEnvelope::looks_like_server_ack(const std::vector<char>& payload) noexcept
{
    if (payload.size() != ServerAckPayloadBytes)
        return false;
    if (static_cast<unsigned char>(payload[0]) != KindServerAck)
        return false;
    return true;
}


} // namespace will
