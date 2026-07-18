#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include "wiremessage_codec.h"


namespace will {


class WireMessageCodec::Internal {
public:
    static constexpr std::uint32_t MaxAuthFieldBytes = 4096;

    static void append_u32_be(std::vector<char>& out, std::uint32_t value);
    static std::uint32_t read_u32_be_at(const unsigned char* data) noexcept;

    static void append_u64_be(std::vector<char>& out, std::uint64_t value);
    static std::uint64_t read_u64_be(const unsigned char* data) noexcept;

    static void append_length_prefixed_string(std::vector<char>& out, std::string_view value);
    static bool read_length_prefixed_string(std::string_view& field, const std::vector<char>& payload,
                                            std::size_t& offset);
    static bool read_length_prefixed_string_allow_empty(std::string_view& field, const std::vector<char>& payload,
                                                        std::size_t& offset);

    static std::string format_user_chat_body_for_log(std::string_view body);

    Internal() = delete;
};


} // namespace will
