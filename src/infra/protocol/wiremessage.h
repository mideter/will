#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "wiremessage_base.h"
#include "wiremessage_client.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"


namespace will {


struct UserChat {
    std::string body;

    bool operator==(const UserChat& other) const noexcept { return body == other.body; }
};


struct ServerReceiptAck {
    bool operator==(const ServerReceiptAck&) const noexcept { return true; }
};


struct HistoryRequest {
    std::uint32_t limit = 0;

    bool operator==(const HistoryRequest& other) const noexcept { return limit == other.limit; }
};


struct HistoryItemPayload {
    std::uint64_t message_id = 0;
    bool is_mine = false;
    std::string body;

    bool operator==(const HistoryItemPayload& other) const noexcept
    {
        return message_id == other.message_id && is_mine == other.is_mine && body == other.body;
    }
};


struct HistoryEnd {
    bool operator==(const HistoryEnd&) const noexcept { return true; }
};


struct LoginRequestPayload {
    std::string login;
    std::string password;

    bool operator==(const LoginRequestPayload& other) const noexcept
    {
        return login == other.login && password == other.password;
    }
};


struct LoginResponsePayload {
    bool success = false;
    std::string token;
    std::uint8_t error_code = 0;

    bool operator==(const LoginResponsePayload& other) const noexcept
    {
        return success == other.success && token == other.token && error_code == other.error_code;
    }
};


struct BindToken {
    std::string token;

    bool operator==(const BindToken& other) const noexcept { return token == other.token; }
};


struct AuthRequired {
    bool operator==(const AuthRequired&) const noexcept { return true; }
};


/** Typed payload inside {@link TcpFrame}: first byte {@code WireMessageType}, then type-specific body. */
using WireMessage = std::variant<
    UserChat,
    ServerReceiptAck,
    HistoryRequest,
    HistoryItemPayload,
    HistoryEnd,
    LoginRequestPayload,
    LoginResponsePayload,
    BindToken,
    AuthRequired>;


std::vector<char> encode(const WireMessage& message);
std::optional<WireMessage> decode(const std::vector<char>& payload);

std::vector<char> encode(const WireMessageBase& message);
std::unique_ptr<WireMessageBase> decode_message(const std::vector<char>& payload);

bool is_client_to_server(const WireMessage& message) noexcept;

/** Structurally valid client → server types (auth gating is enforced in the adapter). */
bool is_valid_client_to_server_payload(const std::vector<char>& payload) noexcept;

std::string format_for_log(const WireMessage& message);
std::string format_for_log(const WireMessageBase& message);
std::string format_for_log(const std::vector<char>& payload);


} // namespace will
