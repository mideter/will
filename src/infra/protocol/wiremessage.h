#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "tcpframe.h"


namespace will {


class WireMessage {
public:
    enum class Type : std::uint8_t {
        UserChat = 1,
        ServerReceiptAck = 2,
        HistoryRequest = 3,
        HistoryItem = 4,
        HistoryEnd = 5,
        BindToken = 8,
        AuthRequired = 9,
        AuthOk = 12,
    };

    virtual ~WireMessage() = default;

    virtual Type type() const noexcept = 0;
    virtual std::vector<char> encode() const = 0;
    virtual std::string format_for_log() const = 0;
};


class ClientMessage : public virtual WireMessage {};


class ServerMessage : public virtual WireMessage {};


} // namespace will
