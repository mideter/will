#pragma once

#include <memory>
#include <string>

#include "wiremessage.h"


namespace will {


class UserChatMessage final : public ClientMessage, public ServerMessage {
public:
    explicit UserChatMessage(std::string body = {});

    const std::string& body() const noexcept { return body_; }

    WireMessage::Type type() const noexcept override;
    std::vector<char> encode() const override;
    std::string format_for_log() const override;
    void accept(ClientMessageVisitor& visitor) const override;
    void accept(ServerMessageVisitor& visitor) const override;

    static std::unique_ptr<UserChatMessage> from_bytes(const std::vector<char>& payload);

    bool operator==(const UserChatMessage& other) const noexcept;

private:
    std::string body_;
};


} // namespace will
