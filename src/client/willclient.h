#pragma once

#include <asio.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "clientconfig.h"
#include "wiremessage.h"


namespace will {


/** Server → client: single-byte {@link WireMessage::ServerReceiptAck} frame. */
struct ServerReceiptAck {};

/** Server → client: end of history batch. */
struct HistoryEnd {};

/** Server → client: chat/history requires BindToken on this connection. */
struct AuthRequired {};

/** Peer chat text (UTF-8) after stripping {@link WireMessage::UserChat} prefix. */
using InboundMessage =
    std::variant<ServerReceiptAck, std::string, HistoryItemPayload, HistoryEnd, AuthRequired>;


// TCP: TcpFrame; payload is typed wire message (see wiremessage.h).
class WillClient {
public:
    WillClient();
    explicit WillClient(ClientConfig config);

    void connect();

    /** Login + {@link WireMessage::BindToken} on the current TCP session. */
    void authenticate(std::string_view login, std::string_view password) const;

    /** Sends {@code UserChat} with UTF-8 body (requires prior {@link #authenticate}). */
    void send(std::string_view utf8_chat_body) const;

    /** Sends {@code HistoryRequest} with the given limit; returns false when limit is 0. */
    bool requestHistory(std::uint32_t limit) const;

    /** std::nullopt = peer closed before next frame header; otherwise typed inbound message. */
    std::optional<InboundMessage> receiveMessage() const;

    void shutdown() const;

    const ClientConfig& config() const noexcept;

private:
    std::vector<char> receivePayload() const;

    asio::io_context ioc_;
    mutable asio::ip::tcp::socket socket_;
    ClientConfig config_;
};


} // namespace will
