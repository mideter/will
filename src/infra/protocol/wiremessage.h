#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "tcpframe.h"


namespace will {


class LoginRequestMessage;
class BindTokenMessage;
class HistoryRequestMessage;
class LoginResponseMessage;
class AuthRequiredMessage;
class ServerReceiptAckMessage;
class HistoryItemMessage;
class HistoryEndMessage;
class UserChatMessage;


struct ClientMessageVisitor {
    virtual ~ClientMessageVisitor() = default;

    virtual void on(const LoginRequestMessage& message) = 0;
    virtual void on(const BindTokenMessage& message) = 0;
    virtual void on(const HistoryRequestMessage& message) = 0;
    virtual void on(const UserChatMessage& message) = 0;
};


struct ServerMessageVisitor {
    virtual ~ServerMessageVisitor() = default;

    virtual void on(const LoginResponseMessage& message) = 0;
    virtual void on(const AuthRequiredMessage& message) = 0;
    virtual void on(const ServerReceiptAckMessage& message) = 0;
    virtual void on(const HistoryItemMessage& message) = 0;
    virtual void on(const HistoryEndMessage& message) = 0;
    virtual void on(const UserChatMessage& message) = 0;
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


std::vector<char> encode(const WireMessage& message);
std::unique_ptr<WireMessage> decode_message(const std::vector<char>& payload);
std::unique_ptr<ClientMessage> decode_client_message(const std::vector<char>& payload);
std::unique_ptr<ServerMessage> decode_server_message(const std::vector<char>& payload);

std::string format_for_log(const std::vector<char>& payload);


} // namespace will
