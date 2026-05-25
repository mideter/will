#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>


namespace will {


/** Application payload inside {@link TcpFrame}: kind + client_msg_id + body. */
struct FrameEnvelope {
    static constexpr std::uint8_t KindChat = 1;
    static constexpr std::uint8_t KindServerAck = 2;

    /** After kind + id: two big-endian uint32s (target peers, successful sends). */
    static constexpr std::size_t ServerAckBodyBytes = 8;
    static constexpr std::size_t ChatMinPayloadBytes = 1 + 8; // kind + id, body may be empty
    static constexpr std::size_t ServerAckPayloadBytes = ChatMinPayloadBytes + ServerAckBodyBytes;

    static void append_u64_be(unsigned char b[8], std::uint64_t v) noexcept;
    static std::uint64_t read_u64_be(const unsigned char b[8]) noexcept;
    static void append_u32_be(unsigned char b[4], std::uint32_t v) noexcept;
    static std::uint32_t read_u32_be(const unsigned char b[4]) noexcept;

    static std::vector<char> encode_chat(std::uint64_t client_msg_id, std::string_view utf8_body);
    static std::vector<char> encode_server_ack(std::uint64_t client_msg_id,
                                               std::uint32_t target_peer_count,
                                               std::uint32_t success_send_count);

    /** @return false if payload is not a valid v2 chat frame */
    static bool try_decode_chat(const std::vector<char>& payload,
                                std::uint64_t& client_msg_id_out,
                                std::string_view& utf8_body_out) noexcept;

    /** @return false if not a valid server ACK */
    static bool try_decode_server_ack(const std::vector<char>& payload,
                                      std::uint64_t& client_msg_id_out,
                                      std::uint32_t& target_peer_count_out,
                                      std::uint32_t& success_send_count_out) noexcept;

    /** True if first byte matches v2 chat and length is sufficient */
    static bool looks_like_v2_chat(const std::vector<char>& payload) noexcept;

    /** True if this is a complete server ACK frame */
    static bool looks_like_server_ack(const std::vector<char>& payload) noexcept;

    FrameEnvelope() = delete;
};


} // namespace will
