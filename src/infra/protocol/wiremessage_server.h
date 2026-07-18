#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "wiremessage.h"


namespace will {


class AuthRequiredMessage final : public ServerMessage {
public:
    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;

    static std::unique_ptr<AuthRequiredMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const AuthRequiredMessage&) const noexcept { return true; }
};


class AuthOkMessage final : public ServerMessage {
public:
    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;

    static std::unique_ptr<AuthOkMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const AuthOkMessage&) const noexcept { return true; }
};


class ServerReceiptAckMessage final : public ServerMessage {
public:
    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;

    static std::unique_ptr<ServerReceiptAckMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const ServerReceiptAckMessage&) const noexcept { return true; }
};


class HistoryItemMessage final : public ServerMessage {
public:
    HistoryItemMessage() = default;
    HistoryItemMessage(std::uint64_t message_id, bool is_mine, std::string name, std::string body);

    std::uint64_t message_id() const noexcept { return message_id_; }
    bool is_mine() const noexcept { return is_mine_; }
    const std::string& name() const noexcept { return name_; }
    const std::string& body() const noexcept { return body_; }

    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;

    static std::unique_ptr<HistoryItemMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const HistoryItemMessage& other) const noexcept;

private:
    std::uint64_t message_id_ = 0;
    bool is_mine_ = false;
    std::string name_;
    std::string body_;
};


class HistoryEndMessage final : public ServerMessage {
public:
    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;

    static std::unique_ptr<HistoryEndMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const HistoryEndMessage&) const noexcept { return true; }
};


class PingMessage final : public ServerMessage {
public:
    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;

    static std::unique_ptr<PingMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const PingMessage&) const noexcept { return true; }
};


} // namespace will
