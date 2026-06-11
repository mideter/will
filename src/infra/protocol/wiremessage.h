#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "tcpframe.h"


namespace will {


class ClientMessage;
class ServerMessage;


class ClientMessageVisitor {
public:
    virtual ~ClientMessageVisitor() = default;

    virtual void on(const ClientMessage& message) = 0;
};


class ServerMessageVisitor {
public:
    virtual ~ServerMessageVisitor() = default;

    virtual void on(const ServerMessage& message) = 0;
};


class WireMessage {
public:
    enum class Type : std::uint8_t {
        UserChat = 1,
        ServerReceiptAck = 2,
        HistoryRequest = 3,
        HistoryItem = 4,
        HistoryEnd = 5,
        LoginRequest = 6,
        LoginResponse = 7,
        BindToken = 8,
        AuthRequired = 9,
    };

    virtual ~WireMessage() = default;

    virtual Type type() const noexcept = 0;
    virtual std::vector<char> encode() const = 0;
    virtual std::string format_for_log() const = 0;
};


class ClientMessage : public virtual WireMessage {
public:
    virtual void accept(ClientMessageVisitor& visitor) const = 0;
};


class ServerMessage : public virtual WireMessage {
public:
    virtual void accept(ServerMessageVisitor& visitor) const = 0;
};


} // namespace will
