#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "wiremessage.h"


namespace will {


class LoginResponseMessage final : public ServerMessage {
public:
    enum class Error : std::uint8_t {
        InvalidCredentials = 1,
        ExpiredToken = 2,
    };

    LoginResponseMessage() = default;
    LoginResponseMessage(bool success, std::string token, std::uint8_t error_code);

    bool success() const noexcept { return success_; }
    const std::string& token() const noexcept { return token_; }
    std::uint8_t error_code() const noexcept { return error_code_; }

    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;
    void accept(ServerMessageVisitor& visitor) const override;

    static std::unique_ptr<LoginResponseMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const LoginResponseMessage& other) const noexcept;

private:
    bool success_ = false;
    std::string token_;
    std::uint8_t error_code_ = 0;
};


class AuthRequiredMessage final : public ServerMessage {
public:
    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;
    void accept(ServerMessageVisitor& visitor) const override;

    static std::unique_ptr<AuthRequiredMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const AuthRequiredMessage&) const noexcept { return true; }
};


class ServerReceiptAckMessage final : public ServerMessage {
public:
    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;
    void accept(ServerMessageVisitor& visitor) const override;

    static std::unique_ptr<ServerReceiptAckMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const ServerReceiptAckMessage&) const noexcept { return true; }
};


class HistoryItemMessage final : public ServerMessage {
public:
    HistoryItemMessage() = default;
    HistoryItemMessage(std::uint64_t message_id, bool is_mine, std::string body);

    std::uint64_t message_id() const noexcept { return message_id_; }
    bool is_mine() const noexcept { return is_mine_; }
    const std::string& body() const noexcept { return body_; }

    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;
    void accept(ServerMessageVisitor& visitor) const override;

    static std::unique_ptr<HistoryItemMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const HistoryItemMessage& other) const noexcept;

private:
    std::uint64_t message_id_ = 0;
    bool is_mine_ = false;
    std::string body_;
};


class HistoryEndMessage final : public ServerMessage {
public:
    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;
    void accept(ServerMessageVisitor& visitor) const override;

    static std::unique_ptr<HistoryEndMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const HistoryEndMessage&) const noexcept { return true; }
};


} // namespace will
