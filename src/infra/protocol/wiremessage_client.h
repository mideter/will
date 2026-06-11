#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "wiremessage.h"


namespace will {


class LoginRequestMessage final : public ClientMessage {
public:
    LoginRequestMessage() = default;
    LoginRequestMessage(std::string login, std::string password);

    const std::string& login() const noexcept { return login_; }
    const std::string& password() const noexcept { return password_; }

    WireMessageType type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;
    void accept(ClientMessageVisitor& visitor) const override;

    static std::unique_ptr<LoginRequestMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const LoginRequestMessage& other) const noexcept;

private:
    std::string login_;
    std::string password_;
};


class BindTokenMessage final : public ClientMessage {
public:
    explicit BindTokenMessage(std::string token = {});

    const std::string& token() const noexcept { return token_; }

    WireMessageType type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;
    void accept(ClientMessageVisitor& visitor) const override;

    static std::unique_ptr<BindTokenMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const BindTokenMessage& other) const noexcept;

private:
    std::string token_;
};


class HistoryRequestMessage final : public ClientMessage {
public:
    explicit HistoryRequestMessage(std::uint32_t limit = 0);

    std::uint32_t limit() const noexcept { return limit_; }

    WireMessageType type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;
    void accept(ClientMessageVisitor& visitor) const override;

    static std::unique_ptr<HistoryRequestMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const HistoryRequestMessage& other) const noexcept;

private:
    std::uint32_t limit_ = 0;
};


} // namespace will
