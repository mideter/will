#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "wiremessage.h"


namespace will {


class BindTokenMessage final : public ClientMessage {
public:
    explicit BindTokenMessage(std::string token = {});

    const std::string& token() const noexcept { return token_; }

    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;

    static std::unique_ptr<BindTokenMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const BindTokenMessage& other) const noexcept;

private:
    std::string token_;
};


class HistoryRequestMessage final : public ClientMessage {
public:
    explicit HistoryRequestMessage(std::uint32_t limit = 0);

    std::uint32_t limit() const noexcept { return limit_; }

    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;

    static std::unique_ptr<HistoryRequestMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const HistoryRequestMessage& other) const noexcept;

private:
    std::uint32_t limit_ = 0;
};


} // namespace will
