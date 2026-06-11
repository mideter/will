#pragma once

#include <memory>
#include <string>
#include <vector>

#include "wiremessage.h"


namespace will {


/** Encodes and decodes wire message payloads (public protocol API). */
class WireMessageCodec {
public:
    static std::vector<char> encode(const WireMessage& message);
    static std::unique_ptr<WireMessage> decode(const std::vector<char>& payload);
    static std::unique_ptr<ClientMessage> decode_client(const std::vector<char>& payload);
    static std::unique_ptr<ServerMessage> decode_server(const std::vector<char>& payload);
    static std::string format_for_log(const std::vector<char>& payload);

    /** Binary helpers for message encode/decode (library-internal). */
    class Internal;

    WireMessageCodec() = delete;
};


} // namespace will
