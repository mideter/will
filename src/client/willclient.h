#pragma once

#include <asio.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "clientconfig.h"
#include "willmessage.h"


namespace will {


/** Server → client: single-byte {@link WillMessage::ServerReceiptAck} frame. */
struct ServerReceiptAck {};

/** Server → client: end of history batch. */
struct HistoryEnd {};

/** Peer chat text (UTF-8) after stripping {@link WillMessage::UserChat} prefix. */
using InboundMessage = std::variant<ServerReceiptAck, std::string, HistoryItemPayload, HistoryEnd>;


// TCP: TcpFrame; payload is typed Will message (see willmessage.h).
class WillClient {
public:
    WillClient();
    explicit WillClient(ClientConfig config);

    void connect();

    /** Sends {@code UserChat} with UTF-8 body. */
    void send(std::string_view utf8_chat_body) const;

    /** Sends {@code HistoryRequest} with the given limit. */
    void requestHistory(std::uint32_t limit) const;

    /** std::nullopt = peer closed before next frame header; otherwise typed inbound message. */
    std::optional<InboundMessage> receiveMessage() const;

    void shutdown() const;

    const ClientConfig& config() const noexcept;

private:
    asio::io_context ioc_;
    mutable asio::ip::tcp::socket socket_;
    ClientConfig config_;
};


} // namespace will
